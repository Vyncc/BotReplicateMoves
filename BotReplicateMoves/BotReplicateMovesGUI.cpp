#include "pch.h"
#include "BotReplicateMoves.h"


std::string BotReplicateMoves::GetPluginName() {
	return "BotReplicateMoves";
}

void BotReplicateMoves::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> BotReplicateMoves
void BotReplicateMoves::RenderSettings()
{
	ImGui::Columns(2);


	if (ImGui::Button("Open Menu"))
	{
		gameWrapper->Execute([&](GameWrapper* gw)
			{
				cvarManager->executeCommand("togglemenu " + GetMenuName());
			});
	}

	ImGui::Checkbox("Enable Plugin", &activatePlugin);
	ImGui::SameLine();
	ImGui::Checkbox("UsePlayerCar", &UsePlayerCar);

	ImGui::NewLine();


	ImGui::Checkbox("UseTimeLine", &UseTimeLine);
	ImGui::SliderInt("Timeline", &inputsIndex, 0, CurrentShot.GetTicksCount() - 1);

	ImGui::NewLine();


	if (ImGui::Button("Save data"))
	{
		//SaveActualRecord(CurrentShot.ticks);
	}

	if (ImGui::Button("Open Timeline"))
	{
		ImGui::OpenPopup("Timeline");
	}

	ImGui::SameLine();


	ImGui::NewLine();

	if (ImGui::Button("Set binds"))
	{
		gameWrapper->Execute([&](GameWrapper* gw) {
			cvarManager->executeCommand("bind XboxTypeS_DPad_Down \"replicatemoves_startrecording\"");
			cvarManager->executeCommand("bind XboxTypeS_DPad_Left \"replicatemoves_stoprecording\"");
			cvarManager->executeCommand("bind XboxTypeS_DPad_Right \"replicatemoves_playrecord\"");
			cvarManager->executeCommand("bind XboxTypeS_DPad_Up \"replicatemoves_stopplaying\"");
			});
	}
	ImGui::SameLine();
	if (ImGui::Button("reset binds"))
	{
		gameWrapper->Execute([&](GameWrapper* gw) {
			cvarManager->executeCommand("bind XboxTypeS_DPad_Down \"default_down\"");
			cvarManager->executeCommand("bind XboxTypeS_DPad_Left \"default_left\"");
			cvarManager->executeCommand("bind XboxTypeS_DPad_Right \"default_right\"");
			cvarManager->executeCommand("bind XboxTypeS_DPad_Up \"default_up\"");
			});
	}

	ImGui::NewLine();

	if (ImGui::Button("Start recording"))
	{
		recording = true;
	}

	if (ImGui::Button("Stop recording"))
	{
		recording = false;
	}

	ImGui::NewLine();


	if (ImGui::Button("Play"))
	{
		tickCount = 0;
		inputsIndex = 0;
		botTeleported = false;
		playRecord = true;
	}

	if (ImGui::Button("Stop Playing"))
	{
		playRecord = false;
	}

	ImGui::Text("RecordedInputs Count : %d", CurrentShot.GetTicksCount());

	ImGui::NextColumn();

	RenderFileList();

}


void BotReplicateMoves::RenderFileList()
{
	if (!Directory_Or_File_Exists(dataPath)) return;

	for (const auto& file : std::filesystem::directory_iterator(dataPath))
	{
		if (!file.is_directory() && file.path().extension().string() == ".json")
		{
			if (ImGui::Selectable(file.path().filename().string().c_str()))
			{
				//LoadRecord(file.path());
				LOG("Loaded file : {}", file.path().filename().string());
			}
		}
	}
}

void BotReplicateMoves::RenderTimeLine()
{
	if (ImGui::BeginPopupModal("Timeline", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::BeginChild("##Timeline", ImVec2{500.f, 250.f}, true);
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			//draw_list->AddLine();

			ImGui::EndChild();
		}

		ImGui::EndPopup();
	}
}


// Do ImGui rendering here
void BotReplicateMoves::Render()
{
	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_None))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}


	if (ImGui::Button("Load Pack"))
	{
		ImGui::OpenPopup("Load Pack");
	}
	renderLoadPack();

	ImGui::SameLine();

	if (ImGui::Button("Save Pack"))
	{
		ImGui::OpenPopup("Save Pack");
	}
	renderSavePack(CurrentPack);

	ImGui::Separator();


	//for testing
	/*static float width1 = 100.f;
	ImGui::SliderFloat("width1", &width1, 0.f, 20.f);
	static float width2 = 300.f;
	ImGui::SliderFloat("width2", &width2, 0.f, 600.f);*/

	float ShotHeight = 27.f;
	float ShotListHeight = CurrentPack.shots.size() * ShotHeight;

	float ShotNameColWidth = 51.f;
	float ManageColWidth = 169.f;

	auto windowWidth = ImGui::GetWindowSize().x;
	ImGui::SetCursorPosX((ShotNameColWidth + ManageColWidth + 16.f - ImGui::CalcTextSize(CurrentPack.name.c_str()).x) * 0.5f);

	ImGui::Text(CurrentPack.name.c_str());

	ImGui::BeginChild("##ShotList", ImVec2{ ShotNameColWidth + ManageColWidth , ShotListHeight + 8.f }, true);

	ImGui::Columns(2, "ShotList", true);

	ImGui::SetColumnWidth(0, ShotNameColWidth);
	ImGui::SetColumnWidth(1, ManageColWidth);

	for (int n = 0; n < CurrentPack.shots.size(); n++)
	{
		ImGui::PushID(n);

		Shot shot = CurrentPack.shots[n];
		std::string shotName = "Shot " + std::to_string(n);
		ImGui::Text(shotName.c_str());

		ImGui::NextColumn();

		if (ImGui::Button("Edit"))
		{
			CurrentShot = shot;
			selectedShot = n;
			showEditShotWindow = true; //show the edit window
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete"))
		{
			CurrentPack.shots.erase(CurrentPack.shots.begin() + n);
		}
		ImGui::SameLine();
		if (ImGui::Button("Duplicate"))
		{
			CurrentPack.shots.push_back(shot);
		}

		ImGui::NextColumn();

		//if not at the last item of the list
		if (n != CurrentPack.shots.size() - 1)
		{
			ImGui::Separator();
		}

		ImGui::PopID();
	}

	ImGui::EndChild();

	if (ImGui::Button("Play"))
	{
		if (CurrentPack.shots.size() == 0)
			return;

		selectedShot = 0;
		CurrentShot = CurrentPack.shots[selectedShot];
		IsPlayingPack = true;
		gameWrapper->Execute([&](GameWrapper* gw)
			{
				cvarManager->executeCommand("replicatemoves_setshot");
			});
		LOG("Started playing : {}", CurrentPack.name);
	}

	if(ImGui::Button("Add Shot"))
	{
		CurrentPack.shots.push_back(Shot{});
	}

	if (showEditShotWindow)
		RenderEditShotWindow();


	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

void BotReplicateMoves::RenderEditShotWindow()
{
	if (!ImGui::Begin("Edit Shot", &showEditShotWindow, ImGuiWindowFlags_None)) {
		ImGui::End();
		return;
	}

	ImGui::Text("Shot Selected : %d", selectedShot);

	ImGui::NewLine();

	ImGui::Separator();

	for (int n = 0; n < CurrentShot.bots.size(); n++)
	{
		ImGui::PushID(n);
		Bot& bot = CurrentShot.bots[n];

		ImGui::Text("Bot %d", n);

		if (ImGui::Button("Start recording", ImVec2(100.f, 30.f)))
		{
			bot.recording = true;
			if (bot.botIndex != 0)
			{
				tickCount = 0;
				inputsIndex = 0;
				botTeleported = false;
				playRecord = true;
				replaying = false;

				bot.StartEndIndexes.X = 0;

				for (Bot& b : CurrentShot.bots)
				{
					if(b.botIndex != bot.botIndex)
						b.replaying = true;
				}
			}


		}

		ImGui::SameLine();

		if (ImGui::Button("Stop recording", ImVec2(100.f, 30.f)))
		{
			bot.recording = false;
			bot.StartEndIndexes.Y = bot.StartEndIndexes.X + bot.ticks.size() - 1;
			LOG("StartEndIndexes.Y : {}", bot.StartEndIndexes.Y);

			for (Bot& b : CurrentShot.bots)
			{
				b.replaying = false;
			}
		}

		if (ImGui::Button("Delete"))
		{
			CurrentShot.bots.erase(CurrentShot.bots.begin() + n);
		}

		ImGui::Text("Bot Ticks Count : %d", bot.ticks.size());

		ImGui::PopID();


		ImGui::Separator();
	}

	if (ImGui::Button("Add Bot"))
	{
		CurrentShot.bots.push_back(Bot(CurrentShot.bots.size()));
	}

	ImGui::Separator();


	std::string RecordingPlayerInitText = "Start recording player init";
	if (RecordingPlayerInitLoc)
		RecordingPlayerInitText = "Waiting for player init location";

	if (ImGui::Button(RecordingPlayerInitText.c_str(), ImVec2(200.f, 30.f)))
	{
		if (RecordingPlayerInitLoc)
			RecordingPlayerInitLoc = false;
		else
			RecordingPlayerInitLoc = true;
	}

	if (RecordingPlayerInitLoc)
	{
		if (ImGui::Button("Take this init player loc", ImVec2(200.f, 30.f)))
		{
			gameWrapper->Execute([&](GameWrapper* gw)
				{
					cvarManager->executeCommand("replicatemoves_initplayerlocation");
				});
		}
	}


	ImGui::Checkbox("UseTimeLine", &UseTimeLine);
	ImGui::SliderInt("Timeline", &inputsIndex, 0, CurrentShot.GetTicksCount() - 1);


	std::string loc = std::to_string(CurrentShot.InitLocation.X) + ", " + std::to_string(CurrentShot.InitLocation.Y) + ", " + std::to_string(CurrentShot.InitLocation.Z);
	ImGui::Text(loc.c_str());
	std::string rot = std::to_string(CurrentShot.InitRotation.Pitch) + ", " + std::to_string(CurrentShot.InitRotation.Yaw) + ", " + std::to_string(CurrentShot.InitRotation.Roll);
	ImGui::Text(rot.c_str());
	std::string vel = std::to_string(CurrentShot.InitVelocity.X) + ", " + std::to_string(CurrentShot.InitVelocity.Y) + ", " + std::to_string(CurrentShot.InitVelocity.Z);
	ImGui::Text(vel.c_str());

	if (CurrentShot.GetTicksCount() != 0)
	{
		if (ImGui::Button("Play", ImVec2(100.f, 30.f)))
		{
			tickCount = 0;
			inputsIndex = 0;
			botTeleported = false;
			playRecord = true;

			for (Bot& b : CurrentShot.bots)
			{
				b.replaying = true;
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Stop Playing", ImVec2(100.f, 30.f)))
		{
			playRecord = false;
			replaying = false;
			for (Bot& b : CurrentShot.bots)
			{
				b.replaying = false;
			}
		}
	}

	ImGui::NewLine();

	ImGui::Text("Ball Ticks Count : %d", CurrentShot.GetTicksCount());

	ImGui::NewLine();

	if (ImGui::Button("Trim Shot", ImVec2(100.f, 30.f)))
	{
		showTrimShot = true;
	}

	if (showTrimShot)
		renderTrimShot();

	ImGui::NewLine();

	if (ImGui::Button("Save", ImVec2(100.f, 30.f)))
	{
		CurrentPack.shots[selectedShot] = CurrentShot;
		ImGui::OpenPopup("Save");;

		ImGui::CloseCurrentPopup();
	}
	renderInfoPopup("Save", "Shot saved !");

	ImGui::SameLine();
	AlignRightNexIMGUItItem(100.f, 8.f);

	if (ImGui::Button("Close", ImVec2(100.f, 30.f)))
		showEditShotWindow = false;

	ImGui::End();

}

void BotReplicateMoves::renderTrimShot()
{
	if (!ImGui::Begin("Trim Shot", &showTrimShot, ImGuiWindowFlags_None)) {
		ImGui::End();
		return;
	}

	static float widthTest = 100.f;
	static float heightTest = 50.f;
	ImGui::SliderFloat("widthTest", &widthTest, 0.f, 800.f);
	ImGui::SliderFloat("heightTest", &heightTest, 0.f, 800.f);
	ImVec2 size(widthTest, heightTest);

	
	ImGui::Text("inputsIndex : %d", inputsIndex);

	renderTimeLine();


	if (ImGui::Button("Cancel", ImVec2(100.f, 30.f)))
	{
		showTrimShot = false;
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();

	if (ImGui::Button("Cancel", ImVec2(100.f, 30.f)))
	{
		showTrimShot = false;
		ImGui::CloseCurrentPopup();
	}


	ImGui::End();
}

void BotReplicateMoves::renderTimeLine()
{
	//ImGui::Checkbox("Use TimeLine", &UseTimeLine);

	ImVec2 LineOrigin = ImGui::GetCursorScreenPos();
	renderTimeLineBackground();

	renderBallTimeLine();
	for (int i = 0; i < CurrentShot.bots.size(); i++)
	{
		Bot& bot = CurrentShot.bots[i];
		float botTimeLineWidthPercentage = 0.f;
		if (CurrentShot.ballTicks.size() > 0)
			botTimeLineWidthPercentage = float(bot.ticks.size()) / float(CurrentShot.ballTicks.size());
		float botTimeLineWidth = 300.f * botTimeLineWidthPercentage;
		ImGui::PushID(i);
		renderBotTimeLine(std::string("Bot " + std::to_string(i)), ImVec2(botTimeLineWidth, 38.f));
		ImGui::PopID();
	}
	ImVec2 LineEnd = ImGui::GetCursorScreenPos();
	renderLine(LineOrigin, LineEnd);
}

void BotReplicateMoves::renderTimeLineBackground()
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();
	ImVec2 RectFilled_p_max = ImVec2(TopCornerLeft.x + 407.f, TopCornerLeft.y + 30.f + ((CurrentShot.bots.size() + 1) * 42.f));
	draw_list->AddRectFilled(TopCornerLeft, RectFilled_p_max, ImColor(213, 213, 213, 100), 0.f);

	ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 2.f, TopCornerLeft.y + 30.f));

}

void BotReplicateMoves::renderLine(ImVec2 origin, ImVec2 end)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 bottom = ImGui::GetCursorScreenPos();
	float LinePosPercentage = 0.f;
	if (CurrentShot.ballTicks.size() > 0)
		LinePosPercentage = float(inputsIndex) / CurrentShot.ballTicks.size();
	LOG("LinePosPercentage : {}", LinePosPercentage);
	float LinePos = 300.f * LinePosPercentage; //300 = la largeur de la timeLine
	LOG("LinePos : {}", LinePos);
	ImGui::SetCursorScreenPos(ImVec2(origin.x + 100.f + LinePos, origin.y));
	ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();
	ImVec2 RectFilled_p_max = ImVec2(TopCornerLeft.x, end.y);

	draw_list->AddLine(TopCornerLeft, RectFilled_p_max, ImColor(255, 255, 255, 255), 1.f);
	ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x - 100.f - LinePos, bottom.y));
}

void BotReplicateMoves::renderBallTimeLine()
{
	ImGui::BeginGroup();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();
	ImVec2 RectFilled_p_max = ImVec2(TopCornerLeft.x + 94.f, TopCornerLeft.y + 38.f);
	draw_list->AddRectFilled(TopCornerLeft, RectFilled_p_max, ImColor(44, 75, 113, 255), 0.f);

	ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 10.f, TopCornerLeft.y + 10.f));
	ImGui::Text("Ball");
	ImGui::SameLine();
	ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 100.f, TopCornerLeft.y));
	renderRectangle(ImVec2(300.f, 38.f));
	ImGui::EndGroup();

	ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x, RectFilled_p_max.y + 4.f));
}

void BotReplicateMoves::renderBotTimeLine(std::string botName, ImVec2 size)
{
	ImGui::BeginGroup();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();
	ImVec2 RectFilled_p_max = ImVec2(TopCornerLeft.x + 94.f, TopCornerLeft.y + 38.f);
	draw_list->AddRectFilled(TopCornerLeft, RectFilled_p_max, ImColor(44, 75, 113, 255), 0.f);

	ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 10.f, TopCornerLeft.y + 10.f));
	ImGui::Text(botName.c_str());
	ImGui::SameLine();
	ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 100.f, TopCornerLeft.y));
	renderRectangle(ImVec2(size.x, size.y));
	ImGui::EndGroup();

	ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x, RectFilled_p_max.y + 4.f));
}


void BotReplicateMoves::renderRectangle(ImVec2 size)
{
	ImGui::BeginGroup();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();
	ImVec2 RectFilled_p_max = ImVec2(TopCornerLeft.x + size.x, TopCornerLeft.y + size.y);
	draw_list->AddRectFilled(TopCornerLeft, RectFilled_p_max, ImColor(44, 75, 113, 255), 0.f);
	ImGui::EndGroup();
	ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x, RectFilled_p_max.y + 8.f));
}


void BotReplicateMoves::renderSavePack(Pack& pack)
{
	if (ImGui::BeginPopupModal("Save Pack", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Pack Name :");
		ImGui::SameLine();
		static char PackName[64] = "";
		ImGui::InputText("##inpuPackName", PackName, IM_ARRAYSIZE(PackName));

		if (ImGui::IsWindowAppearing())
		{
			strncpy(PackName, pack.name.c_str(), IM_ARRAYSIZE(PackName));
		}


		if (ImGui::Button("Cancel", ImVec2(100.f, 30.f)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		AlignRightNexIMGUItItem(100.f, 8.f);
		if (ImGui::Button("Save", ImVec2(100.f, 25.f)))
		{
			pack.name = std::string(PackName);
			ImGui::OpenPopup("Save?");
		}
		std::string label = "Save " + std::string(PackName) + " ?";
		renderYesNoPopup("Save?", label.c_str(), [this, pack]() {
			SavePack(pack);
			ImGui::CloseCurrentPopup();
			}, [this]() {
				ImGui::CloseCurrentPopup();
			});

		ImGui::EndPopup();
	}
}

void BotReplicateMoves::renderLoadPack()
{
	if (ImGui::BeginPopupModal("Load Pack", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::BeginChild("##PackList", ImVec2{ 200, 200 }, true);

		static std::filesystem::path filePathPopup;

		for (const auto& file : std::filesystem::directory_iterator(dataPath))
		{
			if (ImGui::Selectable(file.path().filename().string().c_str()))
			{
				filePathPopup = file.path();
				ImGui::OpenPopup("Load?");
			}
		}

		std::filesystem::path filePath = filePathPopup;
		std::string label = "Load " + filePath.filename().string() + " ?";
		renderYesNoPopup("Load?", label.c_str(), [this, filePath]() {
			LoadPack(filePath);
			ImGui::CloseCurrentPopup();
			}, [this]() {
				ImGui::CloseCurrentPopup();
			});

		ImGui::EndChild();

		CenterNexIMGUItItem(100.f);
		if (ImGui::Button("Cancel", ImVec2(100.f, 30.f)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void BotReplicateMoves::renderInfoPopup(const char* popupName, const char* label)
{
	if (ImGui::BeginPopupModal(popupName, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(label);
		ImGui::NewLine();
		CenterNexIMGUItItem(100.f);
		if (ImGui::Button("OK", ImVec2(100.f, 25.f)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void BotReplicateMoves::renderYesNoPopup(const char* popupName, const char* label, std::function<void()> yesFunc, std::function<void()> noFunc)
{
	if (ImGui::BeginPopupModal(popupName, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(label);
		ImGui::NewLine();

		CenterNexIMGUItItem(208.f);
		ImGui::BeginGroup();
		{
			if (ImGui::Button("Yes", ImVec2(100.f, 25.f)))
			{
				try
				{
					yesFunc();
				}
				catch (const std::exception& ex)
				{
					cvarManager->log(ex.what());
				}

			}
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(100.f, 25.f)))
			{
				noFunc();
			}

			ImGui::EndGroup();
		}

		ImGui::EndPopup();
	}
}

//https://gist.github.com/dougbinks/ef0962ef6ebe2cadae76c4e9f0586c69
void BotReplicateMoves::renderUnderLine(ImColor col_)
{
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	min.y = max.y;
	ImGui::GetWindowDrawList()->AddLine(min, max, col_, 1.0f);
}

void BotReplicateMoves::CenterNexIMGUItItem(float itemWidth)
{
	auto windowWidth = ImGui::GetWindowSize().x;
	ImGui::SetCursorPosX((windowWidth - itemWidth) * 0.5f);
}

void BotReplicateMoves::AlignRightNexIMGUItItem(float itemWidth, float borderGap)
{
	auto windowWidth = ImGui::GetWindowSize().x;
	float totalWidth = itemWidth + borderGap;
	ImGui::SetCursorPosX(windowWidth - totalWidth);
}

// Name of the menu that is used to toggle the window.
std::string BotReplicateMoves::GetMenuName()
{
	return "BotReplicateMovesMenu";
}

// Title to give the menu
std::string BotReplicateMoves::GetMenuTitle()
{
	return menuTitle_;
}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
bool BotReplicateMoves::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool BotReplicateMoves::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void BotReplicateMoves::OnOpen()
{
	isWindowOpen_ = true;
}

// Called when window is closed
void BotReplicateMoves::OnClose()
{
	isWindowOpen_ = false;
}
