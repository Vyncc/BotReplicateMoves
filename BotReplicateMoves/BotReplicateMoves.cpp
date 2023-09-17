#include "pch.h"
#include "BotReplicateMoves.h"



bool operator==(const Vector& lhs, const Vector& rhs) {
	return lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z;
}

bool operator==(const Rotator& lhs, const Rotator& rhs) {
	return lhs.Pitch == rhs.Pitch && lhs.Yaw == rhs.Yaw && lhs.Roll == rhs.Roll;
}


bool operator==(const Vector2& lhs, const Vector2& rhs) {
	return lhs.X == rhs.X && lhs.Y == rhs.Y;
}

BAKKESMOD_PLUGIN(BotReplicateMoves, "BotReplicateMoves", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void BotReplicateMoves::onLoad()
{
	_globalCvarManager = cvarManager;

	image_play = std::make_shared<ImageWrapper>(dataPath + "\\images\\play.png", false, true);
	image_play_greyed = std::make_shared<ImageWrapper>(dataPath + "\\images\\play_greyed.png", false, true);
	image_pause = std::make_shared<ImageWrapper>(dataPath + "\\images\\pause.png", false, true);
	image_pause_greyed = std::make_shared<ImageWrapper>(dataPath + "\\images\\pause_greyed.png", false, true);
	image_stop = std::make_shared<ImageWrapper>(dataPath + "\\images\\stop.png", false, true);
	image_stop_greyed = std::make_shared<ImageWrapper>(dataPath + "\\images\\stop_greyed.png", false, true);
	image_fastForward = std::make_shared<ImageWrapper>(dataPath + "\\images\\fastforward.png", false, true);
	image_fastBackward = std::make_shared<ImageWrapper>(dataPath + "\\images\\fastbackward.png", false, true);

	image_confirm = std::make_shared<ImageWrapper>(dataPath + "\\images\\confirm.png", false, true);
	image_cancel = std::make_shared<ImageWrapper>(dataPath + "\\images\\cancel.png", false, true);
	image_startRecording = std::make_shared<ImageWrapper>(dataPath + "\\images\\startrecording.png", false, true);
	image_startRecording_Greyed = std::make_shared<ImageWrapper>(dataPath + "\\images\\startrecording_greyed.png", false, true);
	image_stopRecording = std::make_shared<ImageWrapper>(dataPath + "\\images\\stoprecording.png", false, true);
	image_stopRecording_Greyed = std::make_shared<ImageWrapper>(dataPath + "\\images\\stoprecording_greyed.png", false, true);

	image_trim = std::make_shared<ImageWrapper>(dataPath + "\\images\\trim.png", false, true);
	image_trim_Greyed = std::make_shared<ImageWrapper>(dataPath + "\\images\\trim_greyed.png", false, true);
	image_setTrimStart = std::make_shared<ImageWrapper>(dataPath + "\\images\\settrimstart.png", false, true);
	image_setTrimStart_Greyed = std::make_shared<ImageWrapper>(dataPath + "\\images\\settrimstart_greyed.png", false, true);
	image_setTrimEnd = std::make_shared<ImageWrapper>(dataPath + "\\images\\settrimend.png", false, true);
	image_setTrimEnd_Greyed = std::make_shared<ImageWrapper>(dataPath + "\\images\\settrimend_greyed.png", false, true);

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
				StopReplaying();
				LOG("Player hit ball so stop playing the shot");

				
			}
		});


	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState",
		[this](std::string eventname) {
			cvarManager->executeCommand("replicatemoves_nextshot");
		});

	

	gameWrapper->HookEventWithCallerPost<CarWrapper>("Function TAGame.Car_TA.SetVehicleInput", std::bind(&BotReplicateMoves::onTick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Active.StartRound", std::bind(&BotReplicateMoves::InitGame, this, std::placeholders::_1));


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
			playRecord = true;
		}, "", 0);
	cvarManager->registerNotifier("replicatemoves_stopplaying", [&](std::vector<std::string> args)
		{
			playRecord = false;
		}, "", 0);
	cvarManager->registerNotifier("replicatemoves_instantreplay_saveshot", [&](std::vector<std::string> args)
		{
			if (InstantReplayShot.ballTicks.size() > 0)
			{
				Shot shot = InstantReplayShot;
				shot.bots[0].StartEndIndexes.Y = shot.bots[0].StartEndIndexes.X + shot.bots[0].ticks.size() - 1;
				InstantReplayShotList.push_back(shot);
			}
		}, "", 0);
	cvarManager->registerNotifier("replicatemoves_setshot", [&](std::vector<std::string> args)
		{
			if (!IsPlayingPack) return;
			StartReplaying();
			LOG("Shot {} set !", selectedShot);
		}, "", 0);
	cvarManager->registerNotifier("replicatemoves_nextshot", [&](std::vector<std::string> args)
		{
			if (!IsPlayingPack) return;
			if (selectedShot == CurrentPack.shots.size() - 1) //if selectedShot is at the last index
				selectedShot = 0;
			else
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

void BotReplicateMoves::StartReplaying()
{
	if (CurrentShot.GetTicksCount() != 0)
	{
		LOG("Starting to play the shot");

		playRecord = true;
		playingState = PlayingState::SPAWNINGBOT;

		for (Bot& b : CurrentShot.bots)
		{
			b.replaying = true;
		}
	}
}

void BotReplicateMoves::StopReplaying()
{
	if (CurrentShot.GetTicksCount() != 0)
	{
		playRecord = false;
		playingState = PlayingState::STOPPED;

		for (Bot& b : CurrentShot.bots)
		{
			b.replaying = false;
		}
	}
}

bool BotReplicateMoves::ABotIsRecording()
{
	for (Bot& b : CurrentShot.bots)
	{
		if (b.recording)
			return true;
	}
	return false;
}

void BotReplicateMoves::SetPlayerInitPos()
{
	CarWrapper car = gameWrapper->GetLocalCar();
	if (!car) return;

	CurrentShot.playerInit.Location = car.GetLocation();
	CurrentShot.playerInit.Rotation = car.GetRotation();
	CurrentShot.playerInit.Velocity = car.GetVelocity();
	CurrentShot.playerInit.AngularVelocity = car.GetAngularVelocity();

	LOG("Set player init pos !");
}

void BotReplicateMoves::PlayShot(ServerWrapper server)
{
	ArrayWrapper<PriWrapper> pris = server.GetPRIs();
	if (pris.Count() > 1)
	{
		int botCount = 0;
		for (PriWrapper pri : pris)
		{
			if (!pri) continue;
			if (!pri.GetbBot()) continue; //if not bot
			CarWrapper botCar = pri.GetCar();
			if (!botCar) continue;

			for (int n = 0; n < CurrentShot.bots.size(); n++)
			{
				Bot& bot = CurrentShot.bots[n];

				if (bot.botIndex != botCount) continue;
				if (!bot.replaying) continue;

				if (inputsIndex >= bot.StartEndIndexes.X && inputsIndex <= bot.StartEndIndexes.Y)
				{
					BotTick botTick = bot.ticks[inputsIndex - bot.StartEndIndexes.X];
					botCar.SetLocation(botTick.Location);
					//LOG("{} Location : {} | {} | {}", pri.GetPlayerName().ToString(), botTick.Location.X, botTick.Location.Y, botTick.Location.Z);
					botCar.SetRotation(botTick.Rotation);
					botCar.SetVelocity(botTick.Velocity);

					MyControllerInput myinputs = botTick.Input;
					ControllerInput inputs;
					SetInputs(inputs, myinputs);
					botCar.SetInput(inputs);
				}
			}

			botCount++;
		}



		BallWrapper ball = server.GetBall();
		if (!ball.IsNull())
		{
			BallTick ballTick = CurrentShot.ballTicks[inputsIndex];
			ball.SetLocation(ballTick.BallLocation);
			ball.SetRotation(ballTick.BallRotation);
			ball.SetVelocity(ballTick.BallVelocity);
		}

		LOG("replaying.... inputIndex : {}", inputsIndex);

		if (playingState != PlayingState::PAUSED)
		{
			inputsIndex++;
		}
	}
}

void BotReplicateMoves::TeleportBots(ServerWrapper server)
{
	ArrayWrapper<PriWrapper> pris = server.GetPRIs();
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

	LOG("bot teleported !");
}

void BotReplicateMoves::SetupPlayer()
{
	CarWrapper car = gameWrapper->GetLocalCar();
	if (!car) return;

	car.SetLocation(CurrentShot.playerInit.Location);
	car.SetRotation(CurrentShot.playerInit.Rotation);
	car.SetVelocity(CurrentShot.playerInit.Velocity);
	car.SetAngularVelocity(CurrentShot.playerInit.AngularVelocity, false);

	LOG("Player Setup");
}

void BotReplicateMoves::SpawnBots(ServerWrapper server)
{
	//Get how many bots are already in the game
	ArrayWrapper<PriWrapper> pris = server.GetPRIs();
	int botCount = 0;
	for (PriWrapper pri : pris)
	{
		if (!pri) continue;
		if (pri.GetbBot())  //if is bot
			botCount++;
	}

	int BotsToSpawn = CurrentShot.bots.size() - botCount;

	//Spawning missing Bots
	for (int n = 0; n < BotsToSpawn; n++)
	{
		std::string botname = "ReplicatingMyMoves" + std::to_string(n + 1);
		server.SpawnBot(4284, botname);
		cvarManager->log("bot spawned : " + botname);
	}
}

void BotReplicateMoves::onTick(CarWrapper caller, void* params, std::string eventname)
{
	if (!activatePlugin) { return; }

	CarWrapper car = gameWrapper->GetLocalCar();
	if (!car) { return; }
	if (caller.memory_address != car.memory_address) return;

	if (!(gameWrapper->IsInFreeplay() || gameWrapper->IsInCustomTraining() || gameWrapper->IsInGame())) return;

	ServerWrapper sw = gameWrapper->GetCurrentGameState();
	if (sw.IsNull()) { return; }

	//if I don't do this the grame crashes at the end of the match (error : Access violation reading location 0x0000000000000000, cause the pri below(in the for loop))
	if (sw.GetbMatchEnded())
	{
		return;
	}

	if (playRecord && CurrentShot.GetTicksCount() > 0)
	{
		if ((playingState == PlayingState::PLAYING || playingState == PlayingState::PAUSED) && inputsIndex < CurrentShot.GetTicksCount()) //Set recorded inputs to the bots
		{
			LOG("PLAYING");
			PlayShot(sw);
		}
		else if (playingState == PlayingState::TELEPORTINGBOT) //after 200 ticks, bot gets teleported, the shot starts
		{
			if (tickCount > 200)
			{
				LOG("TELEPORTINGBOT");
				TeleportBots(sw);

				if (IsPlayingPack)
				{
					LOG("Setuping Player...");
					SetupPlayer();
				}

				inputsIndex = 0;
				playingState = PlayingState::PLAYING;
			}
			else
			{
				tickCount++;
			}
		}
		else if (playingState == PlayingState::SPAWNINGBOT) //spawns bot
		{
			LOG("SPAWNINGBOT");
			SpawnBots(sw);

			tickCount = 0;
			playingState = PlayingState::TELEPORTINGBOT;
		}
		else if (inputsIndex == CurrentShot.GetTicksCount())
		{
			LOG("STOPPED");
			playRecord = false;

			playingState = PlayingState::STOPPED;

			for (Bot& b : CurrentShot.bots)
			{
				b.replaying = false;
			}
		}
	}






	for (int n = 0; n < CurrentShot.bots.size(); n++)
	{
		Bot& bot = CurrentShot.bots[n];
		if (bot.recording)
		{
			CarWrapper car = gameWrapper->GetLocalCar();
			if (!car) { return; }

			BallWrapper ball = sw.GetBall();
			if (!ball) { return; }


			BallTick ballTick;
			ballTick.BallLocation = ball.GetLocation();
			ballTick.BallRotation = ball.GetRotation();
			ballTick.BallVelocity = ball.GetVelocity();


			ControllerInput inputs = car.GetInput();
			MyControllerInput myinputs;
			SetMyInputs(myinputs, inputs);

			BotTick botTick;
			botTick.Input = myinputs;
			botTick.Location = car.GetLocation();
			botTick.Rotation = car.GetRotation();
			botTick.Velocity = car.GetVelocity();



			if (bot.botIndex == 0)
			{
				bot.ticks.push_back(botTick);
				CurrentShot.ballTicks.push_back(ballTick);
			}
			else if(playingState == PlayingState::PLAYING)
			{
				bot.ticks.push_back(botTick);
				LOG("bot.ticks.size {}, CurrentShot.ballTicks.size {}", bot.ticks.size(), CurrentShot.ballTicks.size());


				if (bot.ticks.size() > CurrentShot.ballTicks.size())
				{
					CurrentShot.ballTicks.push_back(ballTick);
					playRecord = false;
				}
				else if (bot.ticks.size() <= CurrentShot.ballTicks.size() && !playRecord)
				{
					CurrentShot.ballTicks[bot.ticks.size() - 1] = ballTick;
					LOG("Overriding existing ballticks");
				}
			}
		}
	}

	if (InstantReplayEnabled)
	{
		CarWrapper car = gameWrapper->GetLocalCar();
		if (!car) { return; }

		BallWrapper ball = sw.GetBall();
		if (!ball) { return; }


		BallTick ballTick;
		ballTick.BallLocation = ball.GetLocation();
		ballTick.BallRotation = ball.GetRotation();
		ballTick.BallVelocity = ball.GetVelocity();


		ControllerInput inputs = car.GetInput();
		MyControllerInput myinputs;
		SetMyInputs(myinputs, inputs);

		BotTick botTick;
		botTick.Input = myinputs;
		botTick.Location = car.GetLocation();
		botTick.Rotation = car.GetRotation();
		botTick.Velocity = car.GetVelocity();


		if (InstantReplayShot.bots[0].ticks.size() >= InstantReplayLength || InstantReplayShot.ballTicks.size() >= InstantReplayLength)
		{
			InstantReplayShot.bots[0].ticks.erase(InstantReplayShot.bots[0].ticks.begin()); //remove first element
			InstantReplayShot.ballTicks.erase(InstantReplayShot.ballTicks.begin());	//remove first element
		}

		InstantReplayShot.ballTicks.push_back(ballTick);
		InstantReplayShot.bots[0].ticks.push_back(botTick);
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

//void BotReplicateMoves::SaveActualRecord(std::vector<Tick> recordsList)
//{
//	//auto conversion to json
//	json asd_as_json = recordsList;
//
//	time_t now = time(0);
//	tm* ltm = localtime(&now);
//
//	std::string fileName = "Record " + std::to_string(1900 + ltm->tm_year) + "." + std::to_string(1 + ltm->tm_mon) + "." + std::to_string(ltm->tm_mday) + " - " + std::to_string(ltm->tm_hour) + "." + std::to_string(ltm->tm_min) + "." + std::to_string(ltm->tm_sec) + ".json";
//
//	auto out_path = gameWrapper->GetDataFolder() / "BotReplicateMoves" / fileName;
//	create_directories(out_path.parent_path());
//	auto out = std::ofstream(out_path);
//	out << asd_as_json.dump();
//	LOG("saved actual record");
//}
//
//void BotReplicateMoves::LoadRecord(std::filesystem::path filePath)
//{
//	auto in = std::ifstream(filePath);
//	json asd_read_as_json = json::parse(in);
//	CurrentShot.ticks = asd_read_as_json.get<std::vector<Tick>>();
//}

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