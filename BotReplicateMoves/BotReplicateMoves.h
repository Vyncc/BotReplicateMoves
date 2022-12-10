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


struct Record
{
	MyControllerInput Input;
	Vector Location;
	Rotator Rotation;
	Vector Velocity;

	Vector BallLocation;
	Rotator BallRotation;
	Vector BallVelocity;
};

//magic macro that defines serialize\deserialize functions automagically
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MyControllerInput, Throttle, Steer, Pitch, Yaw, Roll, DodgeForward, DodgeStrafe, Handbrake, Jump, ActivateBoost, HoldingBoost, Jumped)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector, X, Y, Z)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Rotator, Pitch, Yaw, Roll)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Record, Input, Location, Rotation, Velocity, BallLocation, BallRotation, BallVelocity)


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



class BotReplicateMoves: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow/*, public BakkesMod::Plugin::PluginWindow*/
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
	bool playRecord = false;
	bool botSpawned = false;
	bool botTeleported = false;

	Vector recordInitLocation;
	Rotator recordInitRotation;
	Vector recordInitVelocity;
	float recordInitBoostAmount = 1.f;

	int tickCount = 0;
	int inputsIndex = 0;

	std::vector<Record> RecordsList;



	void SaveActualRecord(std::vector<Record> recordsList);
	void LoadActualRecord(std::filesystem::path filePath);


	std::string dataPath = gameWrapper->GetDataFolder().string() + "\\BotReplicateMoves\\";


	void onTick(std::string eventName);
	void InitGame(std::string eventName);

	//Canvas
	virtual void RenderCanvas(CanvasWrapper canvas);

	// Inherited via PluginSettingsWindow
	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;

	void RenderFileList();
	void RenderTimeLine();
	

	// Inherited via PluginWindow
	/*

	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "BotReplicateMoves";

	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
	
	*/
};

