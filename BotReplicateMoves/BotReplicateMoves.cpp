#include "pch.h"
#include "BotReplicateMoves.h"


BAKKESMOD_PLUGIN(BotReplicateMoves, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void BotReplicateMoves::onLoad()
{
	_globalCvarManager = cvarManager;

	gameWrapper->RegisterDrawable(std::bind(&BotReplicateMoves::RenderCanvas, this, std::placeholders::_1));

	//gameWrapper->HookEvent("Function ProjectX.PlayerInput_X.GetKeyForActionArray", std::bind(&BotReplicateMoves::onTick, this, std::placeholders::_1));

	//gameWrapper->HookEventPost("Function ProjectX.PlayerInput_X.GetKeyForActionArray", std::bind(&BotReplicateMoves::onTick, this, std::placeholders::_1));

	//gameWrapper->HookEvent("Function TAGame.PlayerInput_TA.GetKeyForAction", std::bind(&BotReplicateMoves::onTick, this, std::placeholders::_1));


	gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.OnHitBall",
		[this](CarWrapper caller, void* params, std::string eventname) {

			ServerWrapper sw = gameWrapper->GetCurrentGameState();
			if (sw.IsNull()) { return; }

			CarWrapper botCar = sw.GetPRIs().Get(sw.GetPRIs().Count() - 1).GetCar();
			if (UsePlayerCar){ botCar = gameWrapper->GetLocalCar(); }
			if (!botCar || !caller) { return; }

			if (botCar.GetOwnerName() != caller.GetOwnerName())
			{
				playRecord = false;
				LOG("a car hit ball so stop playing the record");
			}
		});

	gameWrapper->HookEventPost("Function TAGame.Car_TA.SetVehicleInput", std::bind(&BotReplicateMoves::onTick, this, std::placeholders::_1));

	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Active.StartRound", std::bind(&BotReplicateMoves::InitGame, this, std::placeholders::_1));

	cvarManager->registerNotifier("replicatemoves", [&](std::vector<std::string> args)
		{
			ServerWrapper sw = gameWrapper->GetCurrentGameState();
			if (sw.IsNull()) { return; }

			BallWrapper ball = sw.GetBall();
			ball.SetLocation(Vector{ 0, 0, 92.739998 });
			ball.SetRotation(Rotator{ 0, 0, 0 });
			ball.SetVelocity(Vector{ 0, 0, 0 });

		}, "", 0);

	cvarManager->registerNotifier("test", [&](std::vector<std::string> args)
		{
			ServerWrapper sw = gameWrapper->GetCurrentGameState();
			if (sw.IsNull()) { return; }

			sw.GetPRIs().Get(1).GetCar().GetCollisionComponent().SetBlockRigidBody2(0);

		}, "", 0);


	cvarManager->registerNotifier("replicatemoves_startrecording", [&](std::vector<std::string> args)
		{
			recording = true;

		}, "", 0);
	cvarManager->registerNotifier("replicatemoves_stoprecording", [&](std::vector<std::string> args)
		{
			recording = false;

		}, "", 0);
	cvarManager->registerNotifier("replicatemoves_playrecord", [&](std::vector<std::string> args)
		{
			tickCount = 0;
			inputsIndex = 0;
			botTeleported = false;
			playRecord = true;

		}, "", 0);
	cvarManager->registerNotifier("replicatemoves_stopplaying", [&](std::vector<std::string> args)
		{
			playRecord = false;

		}, "", 0);
}


void BotReplicateMoves::onTick(std::string eventName)
{
	if (!activatePlugin) { return; }

	if (gameWrapper->IsInFreeplay() || gameWrapper->IsInCustomTraining() || gameWrapper->IsInGame())
	{
		ServerWrapper sw = gameWrapper->GetCurrentGameState();
		if (sw.IsNull()) { return; }

		//if I don't do this the grame crashes at the end of the match (error : Access violation reading location 0x0000000000000000, cause the pri below(in the for loop))
		if (sw.GetbMatchEnded())
		{
			return;
		}


		static bool wasRecording = false;

		if (recording)
		{
			CarWrapper car = gameWrapper->GetLocalCar();
			if (!car) { return; }

			BallWrapper ball = sw.GetBall();
			if (!ball) { return; }

			if (!wasRecording)
			{
				cvarManager->log("RecordsList cleared");
				cvarManager->log("Recording...");

				RecordsList.clear();


				wasRecording = true;
				recordInitLocation = car.GetLocation();
				recordInitRotation = car.GetRotation();
				recordInitVelocity = car.GetVelocity();
				BoostWrapper carBoost = car.GetBoostComponent();
				if (!carBoost.IsNull())
				{
					recordInitBoostAmount = carBoost.GetCurrentBoostAmount();
				}
			}


			ControllerInput inputs = car.GetInput();
			MyControllerInput myinputs;
			SetMyInputs(myinputs, inputs);

			Record record;
			record.Input = myinputs;
			record.Location = car.GetLocation();
			record.Rotation = car.GetRotation();
			record.Velocity = car.GetVelocity();

			record.BallLocation = ball.GetLocation();
			record.BallRotation = ball.GetRotation();
			record.BallVelocity = ball.GetVelocity();

			RecordsList.push_back(record);
		}
		else
		{ 
			wasRecording = false;
		}



		if (playRecord && !recording && RecordsList.size() > 0)
		{
			if (botSpawned && botTeleported && inputsIndex < RecordsList.size()) //Set recorded inputs to the bot
			{
				Record record = RecordsList.at(inputsIndex);

				CarWrapper botCar = sw.GetPRIs().Get(sw.GetPRIs().Count() - 1).GetCar();

				if (UsePlayerCar)
				{
					botCar = gameWrapper->GetLocalCar();
				}

				if (!botCar.IsNull())
				{
					botCar.SetLocation(record.Location);
					botCar.SetRotation(record.Rotation);
					botCar.SetVelocity(record.Velocity);

					MyControllerInput myinputs = record.Input;
					ControllerInput inputs;
					SetInputs(inputs, myinputs);
					botCar.SetInput(inputs);
				}
				
				BallWrapper ball = sw.GetBall();
				if (!ball.IsNull())
				{
					ball.SetLocation(record.BallLocation);
					ball.SetRotation(record.BallRotation);
					ball.SetVelocity(record.BallVelocity);
				}


				if (!UseTimeLine)
				{
					inputsIndex++;
				}
			}
			else if (botSpawned && !botTeleported && tickCount > 200) //after 200 ticks, bot gets teleported
			{
				CarWrapper botCar = sw.GetPRIs().Get(sw.GetPRIs().Count() - 1).GetCar();
				if (UsePlayerCar)
				{
					botCar = gameWrapper->GetLocalCar();
				}

				if (!UsePlayerCar)
				{
					botCar.GetAIController().DoNothing();
				}
				botCar.SetRotation(recordInitRotation);
				botCar.SetLocation(recordInitLocation);
				//botCar.SetVelocity(recordInitVelocity);
				BoostWrapper BoostBot = botCar.GetBoostComponent();
				if (!BoostBot.IsNull())
				{
					BoostBot.SetBoostAmount(recordInitBoostAmount);
				}


				cvarManager->log("car body : " + std::to_string(botCar.GetLoadoutBody()));


				cvarManager->log("bot teleported !");
				botTeleported = true;
				inputsIndex = 0;
			}
			else if (inputsIndex == RecordsList.size())
			{
				playRecord = false;
			}
			else if (!botSpawned) //spawns bot
			{
				std::string botname = "ReplicatingMyMoves";
				sw.SpawnBot(4284, botname);
				cvarManager->log("bot spawned : " + botname);
				botSpawned = true;
			}
			tickCount++;
		}
	}
}

void BotReplicateMoves::SetMyInputs(MyControllerInput& myinputs, ControllerInput inputs)
{
	myinputs.Throttle = inputs.Throttle;
	myinputs.Steer = inputs.Steer;
	myinputs.Pitch = inputs.Pitch;
	myinputs.Yaw = inputs.Yaw;
	myinputs.Roll = inputs.Roll;
	myinputs.DodgeForward = inputs.DodgeForward;
	myinputs.DodgeStrafe = inputs.DodgeStrafe;
	myinputs.Handbrake = inputs.Handbrake;
	myinputs.Jump = inputs.Jump;
	myinputs.ActivateBoost = inputs.ActivateBoost;
	myinputs.HoldingBoost = inputs.HoldingBoost;
	myinputs.Jumped = inputs.Jumped;
}
void BotReplicateMoves::SetInputs(ControllerInput& inputs, MyControllerInput myinputs)
{
	inputs.Throttle = myinputs.Throttle;
	inputs.Steer = myinputs.Steer;
	inputs.Pitch = myinputs.Pitch;
	inputs.Yaw = myinputs.Yaw;
	inputs.Roll = myinputs.Roll;
	inputs.DodgeForward = myinputs.DodgeForward;
	inputs.DodgeStrafe = myinputs.DodgeStrafe;
	inputs.Handbrake = myinputs.Handbrake;
	inputs.Jump = myinputs.Jump;
	inputs.ActivateBoost = myinputs.ActivateBoost;
	inputs.HoldingBoost = myinputs.HoldingBoost;
	inputs.Jumped = myinputs.Jumped;
}

void BotReplicateMoves::SaveActualRecord(std::vector<Record> recordsList)
{
	//auto conversion to json
	json asd_as_json = recordsList;

	time_t now = time(0);
	tm* ltm = localtime(&now);

	std::string fileName = "Record " + std::to_string(1900 + ltm->tm_year) + "." + std::to_string(1 + ltm->tm_mon) + "." + std::to_string(ltm->tm_mday) + " - " + std::to_string(ltm->tm_hour) + "." + std::to_string(ltm->tm_min) + "." + std::to_string(ltm->tm_sec) + ".json";

	auto out_path = gameWrapper->GetDataFolder() / "BotReplicateMoves" / fileName;
	create_directories(out_path.parent_path());
	auto out = std::ofstream(out_path);
	out << asd_as_json.dump();
	LOG("saved actual record");
}

void BotReplicateMoves::LoadActualRecord(std::filesystem::path filePath)
{
	auto in = std::ifstream(filePath);
	json asd_read_as_json = json::parse(in);
	RecordsList = asd_read_as_json.get<std::vector<Record>>();
}

void BotReplicateMoves::InitGame(std::string eventName)
{
	ServerWrapper sw = gameWrapper->GetCurrentGameState();
	if (sw.IsNull()) { return; }

	if (sw.GetGameTimeRemaining() != 300.f) { return; }

	cvarManager->log("game started");

	botSpawned = false;

}

void BotReplicateMoves::RenderCanvas(CanvasWrapper canvas)
{
	if (recording)
	{
		canvas.SetPosition(Vector2{ 0, 0 });
		canvas.SetColor(LinearColor{ 0, 255, 0, 255 });
		canvas.DrawString("Recording...");
	}

	if (playRecord)
	{
		canvas.SetPosition(Vector2{ 0, 18 });
		canvas.SetColor(LinearColor{ 0, 255, 0, 255 });
		canvas.DrawString("Playing");
	}

}


void BotReplicateMoves::onUnload()
{
}