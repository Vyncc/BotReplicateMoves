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

			CarWrapper myCar = gameWrapper->GetLocalCar();
			if (!myCar || !caller) { return; }

			if (myCar.memory_address == caller.memory_address) //if the car that hits the ball is the player then we stop replaying the shot
			{
				playRecord = false;
				LOG("Player hit ball so stop playing the shot");

				//if current shot is not last shot we setup the next one
				LOG("selectedShot :  {}", selectedShot);
				if (selectedShot + 1 <= CurrentPack.shots.size() - 1 && !SetupingShot)
				{
					SetupingShot = true;
					if (!IsPlayingPack) return;
					gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
						cvarManager->log("Setuping next shot...");
						selectedShot++;
						CurrentShot = CurrentPack.shots[selectedShot];
						LOG("Next shot : {}", selectedShot);
						cvarManager->executeCommand("replicatemoves_setshot");
						}, 3);
				}
				
			}
		});


	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState",
		[this](std::string eventname) {
			cvarManager->executeCommand("replicatemoves_nextshot");
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
	cvarManager->registerNotifier("replicatemoves_initplayerlocation", [&](std::vector<std::string> args)
		{
			if (RecordingPlayerInitLoc);

			CarWrapper car = gameWrapper->GetLocalCar();
			if (!car) return;

			CurrentShot.InitLocation = car.GetLocation();
			CurrentShot.InitRotation = car.GetRotation();
			CurrentShot.InitVelocity = car.GetVelocity();
			CurrentShot.InitAngularVelocity = car.GetAngularVelocity();

			RecordingPlayerInitLoc = false;
			LOG("Init player location set !");
		}, "", 0);
	cvarManager->registerNotifier("replicatemoves_setshot", [&](std::vector<std::string> args)
		{
			if (!IsPlayingPack) return;
			tickCount = 0;
			inputsIndex = 0;
			botTeleported = false;
			playRecord = true;
			SetupingShot = true;
			LOG("Shot {} set !", selectedShot);
		}, "", 0);
	cvarManager->registerNotifier("replicatemoves_nextshot", [&](std::vector<std::string> args)
		{
			if (!IsPlayingPack) return;
			selectedShot++;
			CurrentShot = CurrentPack.shots[selectedShot];
			LOG("Next shot : {}", selectedShot);
			cvarManager->executeCommand("replicatemoves_setshot");
		}, "", 0);
	cvarManager->registerNotifier("replicatemoves_previousshot", [&](std::vector<std::string> args)
		{
			if (!IsPlayingPack) return;
			selectedShot -= 1;
			CurrentShot = CurrentPack.shots[selectedShot];
			LOG("Previous shot : {}", selectedShot);
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

				CurrentShot.ticks.clear();


				wasRecording = true;
				recordInitLocation = car.GetLocation();
				recordInitRotation = car.GetRotation();
				recordInitVelocity = car.GetVelocity();
				BoostWrapper carBoost = car.GetBoostComponent();
				if (!carBoost.IsNull())
				{
					recordInitBoostAmount = carBoost.GetCurrentBoostAmount();
				}
				else
				{
					recordInitBoostAmount = 100.f;
				}
			}


			ControllerInput inputs = car.GetInput();
			MyControllerInput myinputs;
			SetMyInputs(myinputs, inputs);

			Tick tick;
			PlayerTick playerTick;
			playerTick.Input = myinputs;
			playerTick.Location = car.GetLocation();
			playerTick.Rotation = car.GetRotation();
			playerTick.Velocity = car.GetVelocity();
			playerTick.botIndex = 0;

			tick.BallLocation = ball.GetLocation();
			tick.BallRotation = ball.GetRotation();
			tick.BallVelocity = ball.GetVelocity();

			tick.playersTick.push_back(playerTick);
			CurrentShot.ticks.push_back(tick);
		}
		else
		{ 
			wasRecording = false;
		}




		if (playRecord && !recording && CurrentShot.ticks.size() > 0)
		{
			if (botSpawned && botTeleported && inputsIndex < CurrentShot.ticks.size()) //Set recorded inputs to the bot
			{
				Tick tick = CurrentShot.ticks.at(inputsIndex);
				LOG("InputIndex replay : {}", inputsIndex);

				ArrayWrapper<PriWrapper> pris = sw.GetPRIs();
				if (pris.Count() > 1)
				{
					int botCount = 0;
					for (PriWrapper pri : pris)
					{
						if (!pri) continue;
						if (!pri.GetbBot()) continue; //if not bot
						CarWrapper botCar = pri.GetCar();

						/*if (UsePlayerCar)
						{
							botCar = gameWrapper->GetLocalCar();
						}*/

						if (!botCar.IsNull())
						{
							PlayerTick playerTick;
							bool isGoodBot = false;
							for (int i = 0; i < tick.playersTick.size(); i++)
							{
								PlayerTick p = tick.playersTick[i];
								if (p.botIndex == botCount)
								{
									isGoodBot = true;
									playerTick = p;
									break;
								}
							}

							if (isGoodBot)
							{
								botCar.SetLocation(playerTick.Location);
								LOG("{} Location : {} | {} | {}", pri.GetPlayerName().ToString(), playerTick.Location.X, playerTick.Location.Y, playerTick.Location.Z);
								botCar.SetRotation(playerTick.Rotation);
								botCar.SetVelocity(playerTick.Velocity);

								MyControllerInput myinputs = playerTick.Input;
								ControllerInput inputs;
								SetInputs(inputs, myinputs);
								botCar.SetInput(inputs);
							}
						}

						BallWrapper ball = sw.GetBall();
						if (!ball.IsNull())
						{
							ball.SetLocation(tick.BallLocation);
							ball.SetRotation(tick.BallRotation);
							ball.SetVelocity(tick.BallVelocity);
						}

						botCount++;
					}

					LOG("replaying....");

					if (!UseTimeLine)
					{
						inputsIndex++;
					}
				}
			}
			else if (botSpawned && !botTeleported && tickCount > 200) //after 200 ticks, bot gets teleported, the shot starts
			{
				ArrayWrapper<PriWrapper> pris = sw.GetPRIs();
				if (pris.Count() > 1)
				{
					int botCount = 0;
					for (PriWrapper pri : pris)
					{
						if (!pri) continue;
						if (!pri.GetbBot()) continue; //if not bot
						CarWrapper botCar = pri.GetCar();

						botCar.GetAIController().DoNothing();
					}
				}


				cvarManager->log("bot teleported !");
				botTeleported = true;
				inputsIndex = 0;

				//Setup player
				CarWrapper car = gameWrapper->GetLocalCar();
				if (!car) return;
				
				car.SetLocation(CurrentShot.InitLocation);
				car.SetRotation(CurrentShot.InitRotation);
				car.SetVelocity(CurrentShot.InitVelocity);
				car.SetAngularVelocity(CurrentShot.InitAngularVelocity, false);

				LOG("Player is now setup !");

				SetupingShot = false;
			}
			else if (inputsIndex == CurrentShot.ticks.size())
			{
				playRecord = false;
			}
			else if (!botSpawned) //spawns bot
			{
				std::string botname = "ReplicatingMyMoves";
				sw.SpawnBot(4284, botname);
				cvarManager->log("bot spawned : " + botname);

				for (int n = 0; n < CurrentShot.players.size(); n++)
				{
					std::string botname = "ReplicatingMyMoves" + std::to_string(n);
					sw.SpawnBot(4284, botname);
					cvarManager->log("bot spawned : " + botname);
				}

				botSpawned = true;
			}
			tickCount++;
		}



		for (int n = 0; n < CurrentShot.players.size(); n++)
		{
			Player& player = CurrentShot.players[n];
			if (player.recording && (botSpawned && botTeleported && inputsIndex >= 1)) //if recording and replaying other bots
			{
				CarWrapper car = gameWrapper->GetLocalCar();
				if (!car) { return; }

				BallWrapper ball = sw.GetBall();
				if (!ball) { return; }


				ControllerInput inputs = car.GetInput();
				MyControllerInput myinputs;
				SetMyInputs(myinputs, inputs);

				Tick tick;
				PlayerTick playerTick;
				playerTick.Input = myinputs;
				playerTick.Location = car.GetLocation();
				playerTick.Rotation = car.GetRotation();
				playerTick.Velocity = car.GetVelocity();
				playerTick.botIndex = n + 1;

				tick.playersTick.push_back(playerTick);

				tick.BallLocation = ball.GetLocation();
				tick.BallRotation = ball.GetRotation();
				tick.BallVelocity = ball.GetVelocity();

				if (inputsIndex >= CurrentShot.ticks.size() || !playRecord)
				{
					CurrentShot.ticks.push_back(tick);
					playRecord = false;
					LOG("pushback");
				}
				else
				{
					CurrentShot.ticks[inputsIndex - 1].playersTick.push_back(playerTick);
					LOG("replacing");
				}
			}
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

void BotReplicateMoves::SaveActualRecord(std::vector<Tick> recordsList)
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

void BotReplicateMoves::LoadRecord(std::filesystem::path filePath)
{
	auto in = std::ifstream(filePath);
	json asd_read_as_json = json::parse(in);
	CurrentShot.ticks = asd_read_as_json.get<std::vector<Tick>>();
}

void BotReplicateMoves::SavePack(Pack pack)
{
	//auto conversion to json
	json asd_as_json = pack;

	time_t now = time(0);
	tm* ltm = localtime(&now);

	std::string fileName = "Pack " + std::to_string(1900 + ltm->tm_year) + "." + std::to_string(1 + ltm->tm_mon) + "." + std::to_string(ltm->tm_mday) + " - " + std::to_string(ltm->tm_hour) + "." + std::to_string(ltm->tm_min) + "." + std::to_string(ltm->tm_sec) + ".json";

	auto out_path = gameWrapper->GetDataFolder() / "BotReplicateMoves" / fileName;
	create_directories(out_path.parent_path());
	auto out = std::ofstream(out_path);
	out << asd_as_json.dump();
	LOG("saved actual pack");
}

void BotReplicateMoves::LoadPack(std::filesystem::path filePath)
{
	auto in = std::ifstream(filePath);
	json asd_read_as_json = json::parse(in);
	CurrentPack = asd_read_as_json.get<Pack>();
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

	if (IsPlayingPack)
	{
		std::string shotsTxt = "Shot " + std::to_string(selectedShot + 1) + " / " + std::to_string(CurrentPack.shots.size());

		canvas.SetPosition(Vector2{ 5, 50 });
		canvas.SetColor(LinearColor{ 255, 255, 255, 255 });
		canvas.DrawString(shotsTxt, 8.f, 8.f);
	}

}

bool BotReplicateMoves::Directory_Or_File_Exists(const std::filesystem::path& p, std::filesystem::file_status s)
{
	bool DirectoryExists;
	if (std::filesystem::status_known(s) ? std::filesystem::exists(s) : std::filesystem::exists(p))
		DirectoryExists = true;
	else
		DirectoryExists = false;

	return DirectoryExists;
}


void BotReplicateMoves::onUnload()
{
}