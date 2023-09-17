#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

//for ImClamp()
#include "IMGUI/imgui_internal.h"



bool operator==(const Vector& lhs, const Vector& rhs);
bool operator==(const Rotator& lhs, const Rotator& rhs);
bool operator==(const Vector2& lhs, const Vector2& rhs);

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

	bool operator==(const MyControllerInput& other) const {
		return Throttle == other.Throttle &&
			Steer == other.Steer &&
			Pitch == other.Pitch &&
			Yaw == other.Yaw &&
			Roll == other.Roll &&
			DodgeForward == other.DodgeForward &&
			DodgeStrafe == other.DodgeStrafe &&
			Handbrake == other.Handbrake &&
			Jump == other.Jump &&
			ActivateBoost == other.ActivateBoost &&
			HoldingBoost == other.HoldingBoost &&
			Jumped == other.Jumped;
	}
};

struct BotTick
{
	MyControllerInput Input;
	Vector Location;
	Rotator Rotation;
	Vector Velocity;

	bool operator==(const BotTick& other) const {
		return Input == other.Input &&
			Location == other.Location &&
			Rotation == other.Rotation &&
			Velocity == other.Velocity;
	}
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

	bool operator==(const Bot& other) const {
		return botIndex == other.botIndex &&
			StartEndIndexes == other.StartEndIndexes &&
			ticks == other.ticks;
	}
};

struct BallTick
{
	Vector BallLocation;
	Rotator BallRotation;
	Vector BallVelocity;

	bool operator==(const BallTick& other) const {
		return BallLocation == other.BallLocation &&
			BallRotation == other.BallRotation &&
			BallVelocity == other.BallVelocity;
	}
};

struct PlayerInit
{
	Vector Location;
	Rotator Rotation;
	Vector Velocity;
	Vector AngularVelocity;

	bool operator==(const PlayerInit& other) const {
		return Location == other.Location &&
			Rotation == other.Rotation &&
			Velocity == other.Velocity &&
			AngularVelocity == other.AngularVelocity;
	}
};

struct Shot
{
	int ticksCount = 0;

	PlayerInit playerInit;

	std::vector<Bot> bots = { Bot(0) };
	std::vector<BallTick> ballTicks;

	int GetTicksCount() {
		return ballTicks.size();
	}

	bool operator==(const Shot& other) const {
		return ticksCount == other.ticksCount &&
			bots == other.bots &&
			ballTicks == other.ballTicks;
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
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerInit, Location, Rotation, Velocity, AngularVelocity)
//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Shot, ticksCount, InitLocation, InitRotation, InitVelocity, InitAngularVelocity, bots, ballTicks)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Shot, ticksCount, playerInit, bots, ballTicks)
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
	TELEPORTINGBOT = 4,
	PAUSED = 5
};


//really bad ngl but it works
struct Slider
{
	Slider(int* _value, float _minValue, float _maxValue, ImVec2 _sliderSize = ImVec2(200, 20), ImVec2 _cursorSize = ImVec2(10, 20), ImColor _cursorColor = ImColor(200, 200, 200, 255), ImVec2 _CursorOrigin = ImVec2(-1, -1)) {
		value = _value;
		minValue = _minValue;
		maxValue = _maxValue;
		sliderSize = _sliderSize;
		cursorSize = _cursorSize;
		cursorColor = _cursorColor;
		CursorOrigin = _CursorOrigin;
		cursorPosX = 0.f;
		isDragging = false;
		spaceBetweenMouseCursorAndSliderCursor = 0.f;
	}

	void Draw() {
		ImGuiIO& io = ImGui::GetIO();
		ImVec2 cursorPos = ImGui::GetCursorScreenPos();
		if (CursorOrigin.x != -1 && CursorOrigin.y != -1)
			cursorPos = CursorOrigin;
		ImVec2 rectMax = ImVec2(cursorPos.x + sliderSize.x, cursorPos.y + sliderSize.y);

		int val = *value;
		cursorPosX = (val - minValue) * (sliderSize.x) / (maxValue - minValue);

		ImVec2 CursorRectangleMinPos = cursorPos + ImVec2(cursorPosX - (cursorSize.x / 2.f), 0.f);

		ImGui::SetCursorScreenPos(CursorRectangleMinPos);
		ImGui::InvisibleButton("slider", cursorSize + ImVec2(0.f, cursorSize.y));
		if (ImGui::IsItemHovered()) {
			if (io.MouseClicked[0])
			{
				spaceBetweenMouseCursorAndSliderCursor = io.MousePos.x - (cursorPos.x + cursorPosX);
				isDragging = true;
			}
		}

		if (isDragging) {
			if (io.MouseDown[0]) {
				cursorPosX = io.MousePos.x - cursorPos.x - spaceBetweenMouseCursorAndSliderCursor;
				cursorPosX = ImClamp(cursorPosX, 0.f, sliderSize.x);
				*value = float(minValue + (maxValue - minValue) * (cursorPosX / sliderSize.x));
				LOG("dragging...");
			}
			else {
				isDragging = false;
				LOG("not dragging anymore");
			}
		}


		//ImGui::GetWindowDrawList()->AddRectFilled(cursorPos, rectMax, IM_COL32(255, 100, 100, 255));

		CursorRectangleMinPos = cursorPos + ImVec2(cursorPosX - (cursorSize.x / 2.f), 0.f);
		ImVec2 CursorRectangleMaxPos = cursorPos + ImVec2(cursorPosX + (cursorSize.x / 2.f), cursorSize.y);
		ImGui::GetWindowDrawList()->AddRectFilled(CursorRectangleMinPos, CursorRectangleMaxPos, cursorColor);

		ImVec2 p1 = ImVec2(CursorRectangleMinPos.x, CursorRectangleMinPos.y + cursorSize.y);
		ImVec2 p2 = ImVec2(p1.x + cursorSize.x, p1.y);
		ImVec2 p3 = ImVec2((p1.x + p2.x) / 2.f, p1.y + 15.f);
		ImGui::GetWindowDrawList()->AddTriangleFilled(p1, p2, p3, cursorColor);

		SliderCursorScreenPos = p3;

		/*ImGui::SetCursorScreenPos(ImVec2(rectMax.x, cursorPos.y));
		ImGui::Text("%d", *value);*/
	}


	int* value;
	ImVec2 sliderSize;
	ImVec2 cursorSize;
	ImColor cursorColor;
	ImVec2 CursorOrigin;
	float minValue;
	float maxValue;
	ImVec2 SliderCursorScreenPos;
	float cursorPosX;
	bool isDragging;
	float spaceBetweenMouseCursorAndSliderCursor;
};


class BotReplicateMoves: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow, public BakkesMod::Plugin::PluginWindow
{

	//std::shared_ptr<bool> enabled;

	//Boilerplate
	virtual void onLoad();
	virtual void onUnload();

	std::shared_ptr<ImageWrapper> image_play;
	std::shared_ptr<ImageWrapper> image_play_greyed;
	std::shared_ptr<ImageWrapper> image_pause;
	std::shared_ptr<ImageWrapper> image_pause_greyed;
	std::shared_ptr<ImageWrapper> image_stop;
	std::shared_ptr<ImageWrapper> image_stop_greyed;
	std::shared_ptr<ImageWrapper> image_fastForward;
	std::shared_ptr<ImageWrapper> image_fastBackward;


	std::shared_ptr<ImageWrapper> image_confirm;
	std::shared_ptr<ImageWrapper> image_cancel;
	std::shared_ptr<ImageWrapper> image_startRecording;
	std::shared_ptr<ImageWrapper> image_startRecording_Greyed;
	std::shared_ptr<ImageWrapper> image_stopRecording;
	std::shared_ptr<ImageWrapper> image_stopRecording_Greyed;

	std::shared_ptr<ImageWrapper> image_trim;
	std::shared_ptr<ImageWrapper> image_trim_Greyed;
	std::shared_ptr<ImageWrapper> image_setTrimStart;
	std::shared_ptr<ImageWrapper> image_setTrimStart_Greyed;
	std::shared_ptr<ImageWrapper> image_setTrimEnd;
	std::shared_ptr<ImageWrapper> image_setTrimEnd_Greyed;


	void StartReplaying();
	void StopReplaying();
	bool ABotIsRecording();
	void SetPlayerInitPos();

	void SetMyInputs(MyControllerInput& myinputs, ControllerInput inputs);
	void SetInputs(ControllerInput& inputs, MyControllerInput myinputs);

	PlayingState playingState = PlayingState::STOPPED;

	bool activatePlugin = true;
	bool UsePlayerCar = false;
	bool UseTimeLine = false;

	bool IsTrimming = false;

	bool WaitingForBallToTouchGround = false;
	bool IsPlayingPack = false;

	bool recording = false;
	bool playRecord = false;
	void PlayShot(ServerWrapper server);
	void TeleportBots(ServerWrapper server);
	void SetupPlayer();
	void SpawnBots(ServerWrapper server);


	int tickCount = 0;
	int inputsIndex = 0;

	std::vector<Shot> CurrentShotBackupList;
	Shot CurrentShot;
	int selectedShot;
	Pack CurrentPack = { "OhMonGate", {} };

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
	
	bool renderDisableImageButton(const char* id, bool enableButton, ImTextureID on_image, ImTextureID off_image, ImVec2 size, float padding, ImColor imageColor, ImColor backgroundColor, ImColor backgroundColorHovered);

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

