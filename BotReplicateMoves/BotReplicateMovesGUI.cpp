#include "pch.h"
#include "BotReplicateMoves.h"

#include "CustomWidgets.hpp"
using namespace CustomWidget;


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

	ImGui::Columns(2);

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
			CurrentShotBackupList.push_back(CurrentShot);
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



	ImGui::NextColumn();

	renderInstantReplay();

	ImGui::NextColumn();

	if (showEditShotWindow)
		RenderEditShotWindow();

	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

void BotReplicateMoves::renderInstantReplay()
{
	ImGui::Checkbox("Enable Instant Replay", &InstantReplayEnabled);
	ImGui::SliderInt("Instant replay Length", &InstantReplayLength, 100, 30000);

	ImGui::BeginChild("##InstantReplayShotList", ImVec2(ImGui::GetContentRegionAvailWidth(), 200.f), true);

	for (int n = 0; n < InstantReplayShotList.size(); n++)
	{
		ImGui::PushID(n);
		Shot shot = InstantReplayShotList[n];
		ImGui::Text("Shot %d", n);
		ImGui::SameLine();
		if (ImGui::Button("Add Shot To Current Pack"))
		{
			CurrentPack.shots.push_back(shot);
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete"))
		{
			InstantReplayShotList.erase(InstantReplayShotList.begin() + n);
		}
		ImGui::SameLine();
		if (ImGui::Button("Edit"))
		{
			CurrentShot = shot;
			showEditShotWindow = true; //show the edit window
		}
		ImGui::PopID();
	}

	ImGui::EndChild();
}

void BotReplicateMoves::RenderEditShotWindow()
{
	if (!ImGui::Begin("Edit Shot", &showEditShotWindow, ImGuiWindowFlags_None)) {
		ImGui::End();
		return;
	}

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImGui::Text("Shot Selected : %d", selectedShot);

	ImGui::NewLine();
	ImGui::Separator();

	ImGui::NewLine();



	float cellHeight = 100.f;
	ImColor cellBackgroungColor = ImColor(53, 114, 162, 80);

	ImVec2 CursorPos = ImGui::GetCursorScreenPos();
	float width = ImGui::GetContentRegionAvailWidth();

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScrollbarSize = 8.f; // Set scrollbar size a bit more thin

	ImVec2 RectFilledMax = ImVec2(CursorPos.x + width, CursorPos.y + cellHeight);
	ImGui::GetWindowDrawList()->AddRectFilled(CursorPos, RectFilledMax, cellBackgroungColor);

	ImGui::BeginChild(std::string("TimeLineCellChild").c_str(), ImVec2(width, cellHeight), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse); //ImGuiWindowFlags_HorizontalScrollbar

	ImGui::Columns(3);

	if (CurrentShot.playerInit == PlayerInit())
	{
		ImGui::Text("No Player Init Pos !");
	}
	else
	{
		ImGui::Text("Init Pos Fine !");
	}

	if (ImGui::Button("Set Player Init Pos"))
	{
		gameWrapper->Execute([&](GameWrapper* gw)
			{
				SetPlayerInitPos();
			});
	}


	ImGui::NextColumn();

	bool Paused = (playRecord && playingState == PlayingState::PAUSED);
	bool Stopped = (!playRecord && playingState == PlayingState::STOPPED);
	bool CanStartReplaying = (Paused || Stopped);
	if (renderDisableImageButton("PlayButton", CanStartReplaying, image_play->GetImGuiTex(), image_play_greyed->GetImGuiTex(), ImVec2(40.f, 40.f), 5.f, ImColor(255, 255, 255, 200), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0)))
	{
		if (Paused) //it was paused, playing the record again from where it was paused
		{
			LOG("Unpausing");
			playingState = PlayingState::PLAYING;
		}
		else if (Stopped) //it was not playing, starting from the beginning
		{
			StartReplaying();
		}
	}
	if (ImGui::IsItemHovered()) //hovered by mouse
	{
		if (CanStartReplaying)
		{
			ImGui::BeginTooltip();
			if (Stopped)
				ImGui::Text("Start Replaying");
			else if (Paused)
				ImGui::Text("Resume");
			ImGui::EndTooltip();
		}
	}

	ImGui::SameLine();

	bool Playing = (playRecord && playingState == PlayingState::PLAYING);
	if (renderDisableImageButton("PauseButton", Playing, image_pause->GetImGuiTex(), image_pause_greyed->GetImGuiTex(), ImVec2(40.f, 40.f), 5.f, ImColor(255, 255, 255, 200), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0)))
	{
		LOG("Pausing");
		playingState = PlayingState::PAUSED;
	}
	if (ImGui::IsItemHovered()) //hovered by mouse
	{
		if (Playing)
		{
			ImGui::BeginTooltip();
			ImGui::Text("Pause");
			ImGui::EndTooltip();
		}
	}

	ImGui::SameLine();

	bool StopCheck = (playRecord && playingState != PlayingState::STOPPED);
	if (renderDisableImageButton("StopButton", StopCheck, image_stop->GetImGuiTex(), image_stop_greyed->GetImGuiTex(), ImVec2(40.f, 40.f), 5.f, ImColor(255, 255, 255, 200), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0)))
	{
		StopReplaying();
	}
	if (ImGui::IsItemHovered()) //hovered by mouse
	{
		if (StopCheck)
		{
			ImGui::BeginTooltip();
			ImGui::Text("Stop Replaying");
			ImGui::EndTooltip();
		}
	}



	//ImGui::SameLine();
	ImGui::NextColumn();


	if (CheckboxImage("##TrimCheckboxImage", IsTrimming, image_trim_Greyed->GetImGuiTex(), image_trim->GetImGuiTex() , ImVec2(40.f, 40.f), true))
	{
		LOG("IsTrimming : {}", IsTrimming);
		Trim_StartIndex = 0;
		Trim_EndIndex = CurrentShot.ballTicks.size() - 1;
	}
	if (ImGui::IsItemHovered()) //hovered by mouse
	{
		ImGui::BeginTooltip();
		ImGui::Text("Trim");
		ImGui::EndTooltip();
	}

	ImGui::SameLine();

	if (renderDisableImageButton("SetTrimStartButton", IsTrimming, image_setTrimStart->GetImGuiTex(), image_setTrimStart_Greyed->GetImGuiTex(), ImVec2(40.f, 40.f), 5.f, ImColor(255, 255, 255, 200), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0)))
	{
		Trim_StartIndex = inputsIndex;
	}
	if (ImGui::IsItemHovered()) //hovered by mouse
	{
		ImGui::BeginTooltip();
		ImGui::Text("Set Trim Start At Current Tick");
		ImGui::EndTooltip();
	}

	ImGui::SameLine();

	if (renderDisableImageButton("SetTrimEndButton", IsTrimming, image_setTrimEnd->GetImGuiTex(), image_setTrimEnd_Greyed->GetImGuiTex(), ImVec2(40.f, 40.f), 5.f, ImColor(255, 255, 255, 200), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0)))
	{
		Trim_EndIndex = inputsIndex;
	}
	if (ImGui::IsItemHovered()) //hovered by mouse
	{
		ImGui::BeginTooltip();
		ImGui::Text("Set Trim End At Current Tick");
		ImGui::EndTooltip();
	}

	if (IsTrimming && (Trim_StartIndex != 0 || Trim_EndIndex != CurrentShot.ballTicks.size() - 1))
	{
		ImGui::SameLine();

		static bool confirmButtonHovered = false;
		if (ImageButton("ConfirmButton", image_confirm->GetImGuiTex(), ImVec2(20.f, 20.f), 0.f, ImColor(255, 255, 255, 200), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0), confirmButtonHovered))
		{
			//Start
			if (Trim_StartIndex != 0)
			{
				CurrentShot.ballTicks.erase(CurrentShot.ballTicks.begin(), CurrentShot.ballTicks.begin() + Trim_StartIndex);
				for (Bot& bot : CurrentShot.bots)
					bot.ticks.erase(bot.ticks.begin(), bot.ticks.begin() + Trim_StartIndex);

				LOG("Shot trimmed, it now starts at the index {}", Trim_StartIndex);

				Trim_EndIndex -= Trim_StartIndex;
				Trim_StartIndex = 0;
			}

			//End
			if (Trim_EndIndex != 0)
			{
				CurrentShot.ballTicks.erase(CurrentShot.ballTicks.begin() + Trim_EndIndex, CurrentShot.ballTicks.end());
				for (Bot& bot : CurrentShot.bots)
				{
					if (Trim_EndIndex < bot.ticks.size())
						bot.ticks.erase(bot.ticks.begin() + Trim_EndIndex, bot.ticks.end());
				}

				LOG("Shot trimmed, it now ends at the index {}", Trim_EndIndex);
				Trim_EndIndex = 0;
			}

			StopReplaying();
			CurrentShotBackupList.push_back(CurrentShot);
			inputsIndex = 0;
			IsTrimming = false;
		}
		if (ImGui::IsItemHovered()) //hovered by mouse
		{
			ImGui::BeginTooltip();
			ImGui::Text("Confirm Trim");
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
		static bool cancelButtonHovered = false;
		if (ImageButton("CancelButton", image_cancel->GetImGuiTex(), ImVec2(20.f, 20.f), 0.f, ImColor(255, 255, 255, 200), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0), cancelButtonHovered))
		{
			IsTrimming = false;
			Trim_StartIndex = 0;
			Trim_EndIndex = CurrentShot.ballTicks.size() - 1;
		}
		if (ImGui::IsItemHovered()) //hovered by mouse
		{
			ImGui::BeginTooltip();
			ImGui::Text("Cancel Trim");
			ImGui::EndTooltip();
		}
	}


	ImGui::NextColumn();
	ImGui::NextColumn();

	if (CurrentShot.bots.size() > 0)
	{
		Bot& bot = CurrentShot.bots.back();

		bool isRecording = ABotIsRecording();
		bool CanStartRecording = (!isRecording && Stopped);
		if (renderDisableImageButton("StartRecordingButton", CanStartRecording, image_startRecording->GetImGuiTex(), image_startRecording_Greyed->GetImGuiTex(), ImVec2(40.f, 40.f), 5.f, ImColor(255, 255, 255, 200), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0)))
		{
			CurrentShotBackupList.push_back(CurrentShot);

			bot.recording = true;
			if (bot.botIndex != 0)
			{
				playRecord = true;
				playingState = PlayingState::SPAWNINGBOT;

				bot.StartEndIndexes.X = 0;

				for (Bot& b : CurrentShot.bots)
				{
					if (b.botIndex != bot.botIndex)
						b.replaying = true;
				}
			}
		}
		if (ImGui::IsItemHovered()) //hovered by mouse
		{
			ImGui::BeginTooltip();
			ImGui::Text("Start Recording %s", std::string("Bot " + std::to_string(CurrentShot.bots.size() - 1)));
			ImGui::EndTooltip();
		}

		ImGui::SameLine();

		
		if (renderDisableImageButton("StopRecordingButton", isRecording, image_stopRecording->GetImGuiTex(), image_stopRecording_Greyed->GetImGuiTex(), ImVec2(40.f, 40.f), 5.f, ImColor(255, 255, 255, 200), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0)))
		{
			bot.recording = false;
			bot.StartEndIndexes.Y = bot.StartEndIndexes.X + bot.ticks.size() - 1;
			LOG("StartEndIndexes.Y : {}", bot.StartEndIndexes.Y);

			for (Bot& b : CurrentShot.bots)
			{
				b.replaying = false;
			}
		}
		if (ImGui::IsItemHovered()) //hovered by mouse
		{
			ImGui::BeginTooltip();
			ImGui::Text("Stop Recording");
			ImGui::EndTooltip();
		}


		if (!(CurrentShot == CurrentShotBackupList.back()))
		{
			ImGui::SameLine();

			static bool confirmButtonHovered = false;
			if (ImageButton("ConfirmButton", image_confirm->GetImGuiTex(), ImVec2(40.f, 40.f), 5.f, ImColor(255, 255, 255, 200), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0), confirmButtonHovered))
			{
				CurrentShotBackupList.push_back(CurrentShot);
			}
			if (ImGui::IsItemHovered()) //hovered by mouse
			{
				ImGui::BeginTooltip();
				ImGui::Text("Save Record");
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			static bool cancelButtonHovered = false;
			if (ImageButton("CancelButton", image_cancel->GetImGuiTex(), ImVec2(40.f, 40.f), 5.f, ImColor(255, 255, 255, 200), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0), cancelButtonHovered))
			{
				CurrentShot = CurrentShotBackupList.back();
			}
			if (ImGui::IsItemHovered()) //hovered by mouse
			{
				ImGui::BeginTooltip();
				ImGui::Text("Erase Record");
				ImGui::EndTooltip();
			}
		}
	}

	
	ImGui::EndChild();
	style.ScrollbarSize = 16.f; // Set scrollbar size to default



	std::vector<Table::Column> columns = { Table::Column("Name", 0.15f), Table::Column("TimeLine", 0.85f) };
	Table table(columns, ImGui::GetContentRegionAvailWidth());

	table.BeginRow(0);

	table.BeginCell(columns[0].width, 0);

	ImGui::Text("TimeLine");

	table.EndCell();

	ImGui::SameLine(0.f, table.spaceBetweenColumns);

	ImGui::SameLine(0.f, table.spaceBetweenColumns);

	table.BeginCell(columns[1].width, 1);

	/*ImGui::Text("value1 : %d", inputsIndex);
	ImGui::SameLine();
	ImGui::Text("ticks : %d", CurrentShot.ballTicks.size() - 1);*/

	ImVec2 SlidersOrigin = ImGui::GetCursorScreenPos();

	static Slider slider1(&inputsIndex, 0.f, 10.f, ImVec2(ImGui::GetContentRegionAvailWidth(), 28.f), ImVec2(20.f, 10.f), ImColor(200, 200, 200, 255), SlidersOrigin);
	slider1.CursorOrigin = SlidersOrigin;
	slider1.sliderSize.x = ImGui::GetContentRegionAvailWidth();
	if (CurrentShot.ballTicks.size() > 0)
		slider1.maxValue = CurrentShot.ballTicks.size();
	slider1.Draw();
	if (slider1.isDragging || ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("%d", inputsIndex);
		ImGui::EndTooltip();
	}


	static Slider slider2(&Trim_StartIndex, 0.f, 10.f, ImVec2(ImGui::GetContentRegionAvailWidth(), 28.f), ImVec2(20.f, 10.f), ImColor(0, 200, 0, 255), SlidersOrigin);
	slider2.CursorOrigin = SlidersOrigin;
	slider2.sliderSize.x = ImGui::GetContentRegionAvailWidth();
	if (CurrentShot.ballTicks.size() > 0)
		slider2.maxValue = CurrentShot.ballTicks.size();
	if (IsTrimming)
	{
		slider2.Draw();
		if (slider2.isDragging || ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%d", Trim_StartIndex);
			ImGui::EndTooltip();
		}
	}


	static Slider slider3(&Trim_EndIndex, 0.f, 10.f, ImVec2(ImGui::GetContentRegionAvailWidth(), 28.f), ImVec2(20.f, 10.f), ImColor(200, 0, 0, 255), SlidersOrigin);
	slider3.CursorOrigin = SlidersOrigin;
	slider3.sliderSize.x = ImGui::GetContentRegionAvailWidth();
	if (CurrentShot.ballTicks.size() > 0)
		slider3.maxValue = CurrentShot.ballTicks.size();
	if (IsTrimming)
	{
		slider3.Draw();
		if (slider3.isDragging || ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%d", Trim_EndIndex);
			ImGui::EndTooltip();
		}
	}

	

	table.EndCell();

	table.EndRow();

	for (int n = 0; n < CurrentShot.bots.size(); n++)
	{
		Bot& bot = CurrentShot.bots[n];
		table.BeginRow(n + 1);

		table.BeginCell(columns[0].width, 0);

		ImGui::Text("Bot %d", bot.botIndex);

		table.EndCell();


		ImGui::SameLine(0.f, table.spaceBetweenColumns);




		ImGui::SameLine(0.f, table.spaceBetweenColumns);

		table.BeginCell(columns[1].width, 1);

		ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();
		float botTimeLineWidthPercentage = 0.f;
		if (CurrentShot.ballTicks.size() > 0)
			botTimeLineWidthPercentage = float(bot.ticks.size()) / float(CurrentShot.ballTicks.size());
		float botTimeLineWidth = ImGui::GetContentRegionAvailWidth() * botTimeLineWidthPercentage;
		ImVec2 RectFilled_p_max = ImVec2(TopCornerLeft.x + botTimeLineWidth, TopCornerLeft.y + table.cellHeight - (8.f * 2.f));
		draw_list->AddRectFilled(TopCornerLeft, RectFilled_p_max, ImColor(44, 75, 113, 255), 0.f);

		table.EndCell();

		table.EndRow();
	}


	float LineHeight = ImGui::GetCursorScreenPos().y - slider1.SliderCursorScreenPos.y - 8.f;
	draw_list->AddLine(slider1.SliderCursorScreenPos, slider1.SliderCursorScreenPos + ImVec2(0, LineHeight), ImColor(255, 255, 255, 255)); //inputsIndex line
	if (IsTrimming)
	{
		draw_list->AddLine(slider2.SliderCursorScreenPos, slider2.SliderCursorScreenPos + ImVec2(0, LineHeight), ImColor(0, 200, 0, 255)); //Trim_StartIndex line
		draw_list->AddLine(slider3.SliderCursorScreenPos, slider3.SliderCursorScreenPos + ImVec2(0, LineHeight), ImColor(200, 0, 0, 255)); //Trim_EndIndex line
	}

	if (ImGui::Button("Remove Bot", ImVec2(ImGui::GetContentRegionAvailWidth(), 25.f)))
	{
		CurrentShot.bots.pop_back();
	}
	if (ImGui::Button("Add Bot", ImVec2(ImGui::GetContentRegionAvailWidth(), 25.f)))
	{
		CurrentShot.bots.push_back(Bot(CurrentShot.bots.size()));
		CurrentShotBackupList.push_back(CurrentShot);
	}






	ImGui::Separator();
	/*ImGui::Separator();



	ImGui::NewLine();

	ImGui::Text("Ball Ticks Count : %d", CurrentShot.GetTicksCount());

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::NewLine();

	renderTrimShot();

	ImGui::Separator();*/

	ImGui::NewLine();

	if (ImGui::Button("Save", ImVec2(100.f, 30.f)))
	{
		CurrentPack.shots[selectedShot] = CurrentShot;
		ImGui::OpenPopup("Save");

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
	/*static float widthTest = 100.f;
	static float heightTest = 50.f;
	ImGui::SliderFloat("widthTest", &widthTest, 0.f, 800.f);
	ImGui::SliderFloat("heightTest", &heightTest, 0.f, 800.f);
	ImVec2 size(widthTest, heightTest);*/

	
	ImGui::Text("inputsIndex : %d", inputsIndex);

	renderTimeLine();


	ImGui::NewLine();

	ImGui::Checkbox("Pause", &UseTimeLine);
	ImGui::SliderInt("Timeline", &inputsIndex, 0, CurrentShot.GetTicksCount() - 1);

	if (ImGui::Button("Set Begin"))
	{
		Trim_StartIndex = inputsIndex;
	}
	ImGui::SameLine();
	if (ImGui::Button("Set End"))
	{
		Trim_EndIndex = inputsIndex;
	}

	if (ImGui::Button("Trim"))
	{
		//Start
		if (Trim_StartIndex != 0)
		{
			CurrentShot.ballTicks.erase(CurrentShot.ballTicks.begin(), CurrentShot.ballTicks.begin() + Trim_StartIndex);
			for (Bot& bot : CurrentShot.bots)
				bot.ticks.erase(bot.ticks.begin(), bot.ticks.begin() + Trim_StartIndex);

			LOG("Shot trimmed, it now starts at the index {}", Trim_StartIndex);

			Trim_EndIndex -= Trim_StartIndex;
			Trim_StartIndex = 0;
		}

		//End
		if (Trim_EndIndex != 0)
		{
			CurrentShot.ballTicks.erase(CurrentShot.ballTicks.begin() + Trim_EndIndex, CurrentShot.ballTicks.end());
			for (Bot& bot : CurrentShot.bots)
			{
				if(Trim_EndIndex < bot.ticks.size())
					bot.ticks.erase(bot.ticks.begin() + Trim_EndIndex, bot.ticks.end());
			}

			LOG("Shot trimmed, it now ends at the index {}", Trim_EndIndex);
			Trim_EndIndex = 0;
		}
	}

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

	//inputsIndex line
	float LinePosPercentage = 0.f;
	if (CurrentShot.ballTicks.size() > 0)
		LinePosPercentage = float(inputsIndex) / CurrentShot.ballTicks.size();
	LOG("inputsIndex LinePosPercentage : {}", LinePosPercentage);
	renderLine(LineOrigin, LineEnd, LinePosPercentage, ImColor(255, 255, 255, 255));

	static int startIndex = 0;
	static int endIndex = 0;

	//Trim_StartIndex line
	LinePosPercentage = 0.f;
	if(Trim_StartIndex > 0)
		LinePosPercentage = float(Trim_StartIndex) / CurrentShot.ballTicks.size();
	else
		LinePosPercentage = float(startIndex) / CurrentShot.ballTicks.size();
	LOG("Trim_StartIndex LinePosPercentage : {}", LinePosPercentage);
	renderLine(LineOrigin, LineEnd, LinePosPercentage, ImColor(0, 255, 0, 255));

	//Trim_EndIndex line
	LinePosPercentage = 0.f;
	if (CurrentShot.ballTicks.size() > 0)
		endIndex = CurrentShot.ballTicks.size() - 1;

	if (Trim_EndIndex > 0)
		LinePosPercentage = float(Trim_EndIndex) / CurrentShot.ballTicks.size();
	else
		LinePosPercentage = float(endIndex) / CurrentShot.ballTicks.size();
	LOG("Trim_EndIndex LinePosPercentage : {}", LinePosPercentage);
	renderLine(LineOrigin, LineEnd, LinePosPercentage, ImColor(255, 0, 0, 255));

}

void BotReplicateMoves::renderTimeLineBackground()
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();
	ImVec2 RectFilled_p_max = ImVec2(TopCornerLeft.x + 407.f, TopCornerLeft.y + 30.f + ((CurrentShot.bots.size() + 1) * 42.f));
	draw_list->AddRectFilled(TopCornerLeft, RectFilled_p_max, ImColor(213, 213, 213, 100), 0.f);

	ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 2.f, TopCornerLeft.y + 30.f));

}

void BotReplicateMoves::renderLine(ImVec2 origin, ImVec2 end, float posPercentage, ImColor color)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 bottom = ImGui::GetCursorScreenPos();
	float LinePos = 300.f * posPercentage; //300 = la largeur de la timeLine
	LOG("LinePos : {}", LinePos);
	ImGui::SetCursorScreenPos(ImVec2(origin.x + 100.f + LinePos, origin.y));
	ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();
	ImVec2 RectFilled_p_max = ImVec2(TopCornerLeft.x, end.y);

	draw_list->AddLine(TopCornerLeft, RectFilled_p_max, color, 1.f);
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

bool BotReplicateMoves::renderDisableImageButton(const char* id, bool enableButton, ImTextureID on_image, ImTextureID off_image, ImVec2 size, float padding, ImColor imageColor, ImColor backgroundColor, ImColor backgroundColorHovered)
{
	ImVec2 BackgroundMin = ImGui::GetCursorScreenPos();
	ImVec2 BackGroundMax(BackgroundMin.x + size.x, BackgroundMin.y + size.y);

	ImVec2 ImageSize(size.x - padding, size.y - padding);
	ImVec2 ImageMin(BackgroundMin.x + padding, BackgroundMin.y + padding);
	ImVec2 ImageMax(ImageMin.x + ImageSize.x - padding, ImageMin.y + ImageSize.y - padding);

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImGui::BeginChild(id, size, false);
	ImGui::EndChild();

	if (ImGui::IsItemHovered())
	{
		if (enableButton)
		{
			backgroundColor = backgroundColorHovered; //item hovered color
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			if (ImGui::IsMouseClicked(0))
				return true;
		}
	}

	drawList->AddRectFilled(BackgroundMin, BackGroundMax, backgroundColor, 5.f); //background

	if(enableButton)
		drawList->AddImage(on_image, ImageMin, ImageMax, ImVec2(0, 0), ImVec2(1, 1), imageColor);
	else
		drawList->AddImage(off_image, ImageMin, ImageMax, ImVec2(0, 0), ImVec2(1, 1), imageColor);

	return false;
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
