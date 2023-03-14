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

struct Player
{
	bool recording = false;
};

struct PlayerTick
{
	MyControllerInput Input;
	Vector Location;
	Rotator Rotation;
	Vector Velocity;

	int botIndex = 0;
};


struct Tick
{
	std::vector<PlayerTick> playersTick;

	Vector BallLocation;
	Rotator BallRotation;
	Vector BallVelocity;
};

struct Shot
{
	Vector InitLocation = { 0, 0, 0 };
	Rotator InitRotation = { 0, 0, 0 };
	Vector InitVelocity = { 0, 0, 0 };
	Vector InitAngularVelocity = { 0, 0, 0 };

	std::vector<Tick> ticks;

	std::vector<Player> players;
};

struct Pack
{
	std::string name;
	std::vector<Shot> shots;
};


//magic macro that defines serialize\deserialize functions automagically
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MyControllerInput, Throttle, Steer, Pitch, Yaw, Roll, DodgeForward, DodgeStrafe, Handbrake, Jump, ActivateBoost, HoldingBoost, Jumped)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector, X, Y, Z)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Rotator, Pitch, Yaw, Roll)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Player, recording)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerTick, Input, Location, Rotation, Velocity, botIndex)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Tick, playersTick, BallLocation, BallRotation, BallVelocity)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Shot, InitLocation, InitRotation, InitVelocity, InitAngularVelocity, ticks, players)
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



class BotReplicateMoves: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow, public BakkesMod::Plugin::PluginWindow
{

	//std::shared_ptr<bool> enabled;

	//Boilerplate
	virtual void onLoad();
	virtual void onUnload();

	void SetMyInputs(MyControllerInput& myinputs, ControllerInput inputs);
	void SetInputs(ControllerInput& inputs, MyControllerInput myinputs);

	bool activatePlugin = true;
	bool UsePlayerCar = false;
	bool UseTimeLine = false;
	bool recording = false;
	bool SetupingShot = false;;
	bool RecordingPlayerInitLoc = false;
	bool playRecord = false;
	bool botSpawned = false;
	bool botTeleported = false;

	bool IsPlayingPack = false;

	int RecordingPlayerIndex = 0;

	Vector recordInitLocation;
	Rotator recordInitRotation;
	Vector recordInitVelocity;
	float recordInitBoostAmount = 1.f;

	int tickCount = 0;
	int inputsIndex = 0;

	Shot CurrentShot;
	int selectedShot;
	Pack CurrentPack = { "PackMonGate", {} };




	void SaveActualRecord(std::vector<Tick> recordsList);
	void LoadRecord(std::filesystem::path filePath);
	void SavePack(Pack pack);
	void LoadPack(std::filesystem::path filePath);


	std::string dataPath = gameWrapper->GetDataFolder().string() + "\\BotReplicateMoves\\";


	void onTick(std::string eventName);
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

