#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);



struct MyControllerInput
{
	float Throttle = .0f;
	float Steer = .0f;
	float Pitch = .0f;
	float Yaw = .0f;
	float Roll = .0f;
	float DodgeForward = .0f;
	float DodgeStrafe = .0f;
	unsigned long Handbrake;
	unsigned long Jump;
	unsigned long ActivateBoost;
	unsigned long HoldingBoost;
	unsigned long Jumped;
};

struct BotTick
{
	MyControllerInput Input;
	Vector Location;
	Rotator Rotation;
	Vector Velocity;
};

struct Bot
{
	bool recording = false;
	bool replaying = false;
	int botIndex = 0;
	Vector2 StartEndIndexes = {0, 0};
	std::vector<BotTick> ticks;

	Bot(){}
	Bot(int _botIndex) {
		botIndex = _botIndex;
	}

	/*bool operator==(const Bot& other) const {
		return (recording == other.recording &&
			replaying == other.replaying &&
			botIndex == other.botIndex &&
			StartEndIndexes.X == other.StartEndIndexes.X &&
			StartEndIndexes.Y == other.StartEndIndexes.Y &&
			ticks == other.ticks);
	}
	
	bool operator!=(const Bot& other) const {
		return (recording != other.recording ||
			replaying != other.replaying ||
			botIndex != other.botIndex ||
			StartEndIndexes.X != other.StartEndIndexes.X ||
			StartEndIndexes.Y != other.StartEndIndexes.Y ||
			ticks != other.ticks);
	}*/
};

struct BallTick
{
	Vector BallLocation;
	Rotator BallRotation;
	Vector BallVelocity;
};

struct Shot
{
	int ticksCount = 0;

	/*Vector InitLocation = { 0, 0, 0 };
	Rotator InitRotation = { 0, 0, 0 };
	Vector InitVelocity = { 0, 0, 0 };
	Vector InitAngularVelocity = { 0, 0, 0 };*/

	std::vector<Bot> bots = { Bot(0) };
	std::vector<BallTick> ballTicks;

	int GetTicksCount() {
		return ballTicks.size();
	}
};

struct Pack
{
	std::string name;
	std::vector<Shot> shots;
};


//magic macro that defines serialize\deserialize functions automagically
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MyControllerInput, Throttle, Steer, Pitch, Yaw, Roll, DodgeForward, DodgeStrafe, Handbrake, Jump, ActivateBoost, HoldingBoost, Jumped)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector, X, Y, Z)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector2, X, Y)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Rotator, Pitch, Yaw, Roll)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BotTick, Input, Location, Rotation, Velocity)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Bot, recording, replaying, botIndex, StartEndIndexes, ticks)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BallTick, BallLocation, BallRotation, BallVelocity)
//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Shot, ticksCount, InitLocation, InitRotation, InitVelocity, InitAngularVelocity, bots, ballTicks)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Shot, ticksCount, bots, ballTicks)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Pack, name, shots)

/*
void to_json(json& j, const Record& r)
{
	j = json{
		{ "Input", r.Input},
		{ "Location", r.Location},
		{ "Rotation", r.Rotation},
		{ "Velocity", r.Velocity},
		{ "BallLocation", r.BallLocation},
		{ "BallRotation", r.BallRotation},
		{ "BallVelocity", r.BallVelocity}
	};

}

void from_json(const json& j, Record& record)
{
	j.at("Input").get_to(record.Input);
	j.at("Location").get_to(record.Location);
	j.at("Rotation").get_to(record.Rotation);
	j.at("Velocity").get_to(record.Velocity);
	j.at("BallLocation").get_to(record.BallLocation);
	j.at("BallRotation").get_to(record.BallRotation);
	j.at("BallVelocity").get_to(record.BallVelocity);
}
*/

enum RecordingNewAttackerState
{
	NOTRECORDING = -1,
	SETUPINGBOT = 0,
	RECORDING = 1
};

enum class PlayingState
{
	STOPPED = 1,
	PLAYING = 2,
	SPAWNINGBOT = 3,
	TELEPORTINGBOT = 4
};


class BotReplicateMoves: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow, public BakkesMod::Plugin::PluginWindow
{

	//std::shared_ptr<bool> enabled;

	//Boilerplate
	virtual void onLoad();
	virtual void onUnload();

	void SetMyInputs(MyControllerInput& myinputs, ControllerInput inputs);
	void SetInputs(ControllerInput& inputs, MyControllerInput myinputs);

	PlayingState playingState = PlayingState::STOPPED;

	bool activatePlugin = true;
	bool UsePlayerCar = false;
	bool UseTimeLine = false;

	bool SetupingShot = false;
	bool IsPlayingPack = false;

	bool recording = false;
	bool playRecord = false;
	void PlayShot(ServerWrapper server);
	void TeleportBots(ServerWrapper server);
	void SpawnBots(ServerWrapper server);


	int tickCount = 0;
	int inputsIndex = 0;

	Shot CurrentShot;
	int selectedShot;
	Pack CurrentPack = { "PackMonGate", {} };

	Shot InstantReplayShot;
	std::vector<Shot> InstantReplayShotList;
	bool InstantReplayEnabled = false;
	int InstantReplayLength = 1200;
	void renderInstantReplay();


	//void SaveActualRecord(std::vector<Tick> recordsList);
	//void LoadRecord(std::filesystem::path filePath);
	void SavePack(Pack pack);
	void LoadPack(std::filesystem::path filePath);


	std::string dataPath = gameWrapper->GetDataFolder().string() + "\\BotReplicateMoves\\";


	void onTick(CarWrapper caller, void* params, std::string eventname);
	void InitGame(std::string eventName);

	bool Directory_Or_File_Exists(const std::filesystem::path& p, std::filesystem::file_status s = std::filesystem::file_status{});

	//Canvas
	virtual void RenderCanvas(CanvasWrapper canvas);

	// Inherited via PluginSettingsWindow
	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;

	void RenderFileList();
	void RenderTimeLine();

	void RenderEditShotWindow();
	bool showEditShotWindow = false;


	void renderSavePack(Pack& pack);
	void renderLoadPack();


	//Trim Shot
	void renderTrimShot();
	bool showTrimShot = false;
	void renderTimeLine();
	void renderTimeLineBackground();
	void renderLine(ImVec2 origin, ImVec2 end, float posPercentage, ImColor color);
	void renderBotTimeLine(std::string botName, ImVec2 size);
	void renderBallTimeLine();
	void renderRectangle(ImVec2 size);

	int Trim_StartIndex = 0;
	int Trim_EndIndex = 0;
	
	

	// Inherited via PluginWindow
	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "BotReplicateMoves";

	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
	

	//Popups
	void renderInfoPopup(const char* popupName, const char* label);
	void renderYesNoPopup(const char* popupName, const char* label, std::function<void()> yesFunc, std::function<void()> noFunc);
	

	//utils
	void renderUnderLine(ImColor col_);
	void CenterNexIMGUItItem(float itemWidth);
	void AlignRightNexIMGUItItem(float itemWidth, float borderGap);
};

