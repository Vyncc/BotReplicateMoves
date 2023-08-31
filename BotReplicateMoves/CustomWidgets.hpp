#pragma once

#include "pch.h"

#include "IMGUI/imgui_internal.h"

//needed for ShellExecute
#include <Windows.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")



#define M_PI           3.14159265358979323846

#define BUTTON_COLOR_BLUE ImColor(53, 114, 162, 140)
#define BUTTON_COLOR_BLUE_HOVERED ImColor(53, 114, 220, 200)

namespace CustomWidget
{
	double degreeToRadians(double degreeAngle) {
		double radians = (degreeAngle * M_PI) / 180.0;
		return radians;
	}

	std::wstring s2ws(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}


	void AlignRightNexIMGUItItem(float itemWidth, float borderGap = 0.f)
	{
		auto windowWidth = ImGui::GetWindowSize().x;
		float totalWidth = itemWidth + borderGap;
		ImGui::SetCursorPosX(windowWidth - totalWidth);
	}

	void CenterNexIMGUItItem(float itemWidth, float windowWidth = -1)
	{
		float width = ImGui::GetContentRegionAvailWidth();
		ImVec2 cursorPos = ImGui::GetCursorScreenPos();
		if (windowWidth != -1)
		{
			float width2 = std::abs(width - windowWidth);
			width = windowWidth;
			cursorPos = ImVec2(cursorPos.x - width2, cursorPos.y);
		}
		ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + ((width - itemWidth) * 0.5f), cursorPos.y));
	}

	ImVec2 CalcRealTextSize(const char* text, float fontSize = 13.f)
	{
		float TextSizeMultiplier = fontSize / 13.f; //13 is imgui default font size
		ImVec2 TextSize = ImGui::CalcTextSize(text);
		ImVec2 RealTextSize(TextSize.x * TextSizeMultiplier, TextSize.y * TextSizeMultiplier);
		return RealTextSize;
	}

	void InfoPopup(const char* popupName, const char* label)
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

	void YesNoPopup(const char* popupName, const char* label, std::function<void()> yesFunc, std::function<void()> noFunc)
	{
		if (ImGui::BeginPopupModal(popupName, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text(label);
			ImGui::NewLine();
			CenterNexIMGUItItem(208.f);
			ImGui::BeginGroup();
			{
				if (ImGui::Button("YES", ImVec2(100.f, 25.f)))//YES
				{
					try
					{
						yesFunc();
					}
					catch (const std::exception& ex)
					{

					}
				}
				ImGui::SameLine();
				if (ImGui::Button("NO", ImVec2(100.f, 25.f)))//NO
				{
					noFunc();
				}
				ImGui::EndGroup();
			}
			ImGui::EndPopup();
		}
	}

	void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle)
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		float cos_a = cosf(angle);
		float sin_a = sinf(angle);
		ImVec2 pos[4] =
		{
			center + ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
			center + ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
			center + ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
			center + ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)
		};
		ImVec2 uvs[4] =
		{
			ImVec2(0.0f, 0.0f),
			ImVec2(1.0f, 0.0f),
			ImVec2(1.0f, 1.0f),
			ImVec2(0.0f, 1.0f)
		};

		draw_list->AddImageQuad(tex_id, pos[0], pos[1], pos[2], pos[3], uvs[0], uvs[1], uvs[2], uvs[3], ImColor(255, 255, 255, 200));
	}

	bool Button(const char* text, ImVec2 size = ImVec2(0.f, 0.f), float fontSize = 13.f, ImColor backgroundColor = BUTTON_COLOR_BLUE, ImColor backgroundColorHovered = BUTTON_COLOR_BLUE_HOVERED)
	{
		bool clicked = false;

		if (size.x == 0.f && size.y == 0.f)
		{
			ImVec2 textSize = CalcRealTextSize(text, fontSize);
			size = ImVec2(textSize.x + 8.f, textSize.y + 8.f);
		}

		ImFont* fontA = ImGui::GetDefaultFont();

		ImVec2 BackgroundMin = ImGui::GetCursorScreenPos();
		ImVec2 BackGroundMax(BackgroundMin.x + size.x, BackgroundMin.y + size.y);
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		ImGui::BeginChild(text, size, false);
		ImGui::EndChild();

		if (ImGui::IsItemHovered())
		{
			backgroundColor = backgroundColorHovered; //item hovered color

			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			if (ImGui::IsMouseClicked(0))
				clicked = true;
		}

		drawList->AddRectFilled(BackgroundMin, BackGroundMax, backgroundColor, 5.f);

		float TextSizeMultiplier = fontSize / 13.f;
		ImVec2 TextSize = CalcRealTextSize(text, fontSize);
		ImVec2 TextPos = ImVec2(BackgroundMin.x + ((size.x - TextSize.x) * 0.5f), BackgroundMin.y + ((size.y - TextSize.y) * 0.5f));
		drawList->AddText(fontA, fontSize, TextPos, ImColor(255, 255, 255, 255), text);

		return clicked;
	}

	bool ImageButton(const char* id, ImTextureID image, ImVec2 size, float padding, ImColor imageColor, ImColor backgroundColor, ImColor backgroundColorHovered, bool& hovered, float rotateImageAngle = 0.f)
	{
		ImVec2 BackgroundMin = ImGui::GetCursorScreenPos();
		ImVec2 BackGroundMax(BackgroundMin.x + size.x, BackgroundMin.y + size.y);

		ImVec2 ImageSize(size.x - padding, size.y - padding);
		ImVec2 ImageMin(BackgroundMin.x + padding, BackgroundMin.y + padding);
		ImVec2 ImageMax(ImageMin.x + ImageSize.x - padding, ImageMin.y + ImageSize.y - padding);

		ImDrawList* drawList = ImGui::GetWindowDrawList();

		ImGui::BeginChild(id, size, false);
		if (hovered)
			backgroundColor = backgroundColorHovered; //item hovered color
		drawList->AddRectFilled(BackgroundMin, BackGroundMax, backgroundColor, 5.f); //Blue rectangle

		if (rotateImageAngle == 0)
			drawList->AddImage(image, ImageMin, ImageMax, ImVec2(0, 0), ImVec2(1, 1), imageColor);
		else
			ImageRotated(image, ImVec2(BackgroundMin.x + ((BackGroundMax.x - BackgroundMin.x) * 0.5f), BackgroundMin.y + ((BackGroundMax.y - BackgroundMin.y) * 0.5f)), ImageSize, degreeToRadians(rotateImageAngle));

		ImGui::EndChild();
		if (ImGui::IsItemHovered())
		{
			hovered = true;
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			if (ImGui::IsMouseClicked(0))
				return true;
		}
		else {
			hovered = false;
		}
		return false;
	}

	bool ImageButton2(ImTextureID texture_id, ImVec2 size)
	{
		bool clicked = false;
		ImGui::BeginGroup();

		ImGui::Image(texture_id, size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1));

		if (ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			if (ImGui::IsMouseClicked(0))
			{
				clicked = true;
			}
		}
		ImGui::EndGroup();

		return clicked;
	}


	bool Checkbox(const char* text, bool& boolVar, float fontSize = 13.f, ImVec2 checkboxSize = ImVec2(20.f, 20.f), ImColor textColor = IM_COL32_WHITE)
	{
		bool clicked = false;
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImFont* font = ImGui::GetDefaultFont();
		ImColor hoveredBackgroungColor(53, 114, 162, 100);
		ImVec2 checkboxRectMin = ImGui::GetCursorScreenPos();
		ImVec2 checkboxRectMax = ImVec2(checkboxRectMin.x + checkboxSize.x, checkboxRectMin.y + checkboxSize.y);
		ImVec2 RealTextSize(CalcRealTextSize(text, fontSize));
		float spaceBetweenCheckboxAndText = 4.f;
		ImVec2 textPos = ImVec2(checkboxRectMax.x + spaceBetweenCheckboxAndText, checkboxRectMin.y + ((checkboxSize.y * 0.5f)) - (RealTextSize.y * 0.5f));
		ImVec2 widgetSize = ImVec2(checkboxSize.x + spaceBetweenCheckboxAndText + RealTextSize.x, checkboxSize.y);


		ImGui::BeginChild(text, widgetSize, false);

		ImGui::EndChild();

		if (ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

			drawList->AddRectFilled(checkboxRectMin, checkboxRectMax, hoveredBackgroungColor); //blue rectangle

			if (ImGui::IsMouseClicked(0))
			{
				boolVar = !boolVar;
				clicked = true;
			}
		}

		if (boolVar)
			drawList->AddRectFilled(checkboxRectMin, checkboxRectMax, ImColor(53, 114, 192, 150), 2.f);

		drawList->AddRect(checkboxRectMin, checkboxRectMax, ImColor(255, 255, 255, 255), 2.f, 15, 0.5f);
		drawList->AddText(font, fontSize, textPos, textColor, text);

		return clicked;
	}


	bool CheckboxImage(const char* id, bool& boolVar, ImTextureID on_image, ImTextureID off_image, ImVec2 imageSize, bool showHoveredRect = false)
	{
		ImVec2 ImageMin(ImGui::GetCursorScreenPos());
		ImVec2 ImageMax(ImageMin.x + imageSize.x, ImageMin.y + imageSize.y);

		ImDrawList* drawList = ImGui::GetWindowDrawList();

		ImGui::BeginChild(id, imageSize, false);
		if (boolVar)
			drawList->AddImage(on_image, ImageMin, ImageMax, ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));
		else
			drawList->AddImage(off_image, ImageMin, ImageMax, ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));
		ImGui::EndChild();
		if (ImGui::IsItemHovered())
		{
			if (showHoveredRect)
				drawList->AddRect(ImageMin, ImageMax, ImColor(255, 255, 255, 120));
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			if (ImGui::IsMouseClicked(0))
			{
				boolVar = !boolVar;
				return true;
			}
		}
		return false;
	}


	bool CheckboxColoredText(const char* text, bool& boolVar, ImColor on_color = ImColor(0, 255, 0, 255), ImColor off_color = ImColor(255, 0, 0, 255), float fontSize = 13.f, bool showHoveredRect = false)
	{
		ImVec2 RectMin(ImGui::GetCursorScreenPos());
		ImVec2 RealTextSize(CalcRealTextSize(text, fontSize));
		ImVec2 RectMax(RectMin.x + RealTextSize.x, RectMin.y + RealTextSize.y);

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImFont* font = ImGui::GetDefaultFont();

		ImGui::BeginChild(text, RealTextSize, false);
		if (boolVar)
			drawList->AddText(font, fontSize, RectMin, on_color, text);
		else
			drawList->AddText(font, fontSize, RectMin, off_color, text);
		ImGui::EndChild();
		if (ImGui::IsItemHovered())
		{
			if (showHoveredRect)
				drawList->AddRect(ImVec2(RectMin.x - 4.f, RectMin.y - 4.f), ImVec2(RectMax.x + 4.f, RectMax.y + 4.f), ImColor(255, 255, 255, 120));
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			if (ImGui::IsMouseClicked(0))
			{
				boolVar = !boolVar;
				return true;
			}
		}
		return false;
	}

	void Text(const char* text, float fontSize = 13.f)
	{
		ImVec2 cursorPos = ImGui::GetCursorScreenPos();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImGui::BeginChild(text, CalcRealTextSize(text, fontSize));
		drawList->AddText(ImGui::GetDefaultFont(), fontSize, cursorPos, ImColor(255, 255, 255, 255), text);
		ImGui::EndChild();
	}

	//https://gist.github.com/dougbinks/ef0962ef6ebe2cadae76c4e9f0586c69
	void UnderLine(ImColor col_ = IM_COL32_WHITE)
	{
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		min.y = max.y;
		ImGui::GetWindowDrawList()->AddLine(min, max, col_, 1.0f);
	}

	void Link(std::string link)
	{
		std::wstring w_LINK = s2ws(link);
		LPCWSTR L_LINK = w_LINK.c_str();

		ImGui::TextColored(ImColor(3, 94, 252, 255), link.c_str());
		UnderLine(ImColor(3, 94, 252, 255));
		if (ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			if (ImGui::IsMouseClicked(0))
			{
				ShellExecute(0, 0, L_LINK, 0, 0, SW_SHOW); //left click, open link in web browser
			}
			else if (ImGui::IsMouseClicked(1)) //right click
			{
				ImGui::OpenPopup("Link");
			}
			UnderLine(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
		}

		if (ImGui::BeginPopup("Link"))
		{
			if (ImGui::Selectable("Copy link to clipboard"))
			{
				ImGui::SetClipboardText(link.c_str());
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}





	struct InfoPopupStruct
	{
		InfoPopupStruct(std::string _popupName) {
			popupName = _popupName;
		}

		void Show(std::string message) {
			label = message;
			show = true;
		}

		void Draw() {
			if (show)
			{
				ImGui::OpenPopup(popupName.c_str());
				show = false;
				LOG("show : {}", show);
			}

			if (ImGui::BeginPopupModal(popupName.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text(label.c_str());
				ImGui::NewLine();
				//Center the OK button
				float width = ImGui::GetContentRegionAvailWidth();
				ImVec2 cursorPos = ImGui::GetCursorScreenPos();
				ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + ((width - 100.f) * 0.5f), cursorPos.y));
				if (ImGui::Button("OK", ImVec2(100.f, 25.f)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

	private:
		std::string popupName;
		std::string label;
		bool show;
	};

	struct YesNoPopupStruct
	{
		YesNoPopupStruct(std::string _popupName, std::function<void()> _yesFunc, std::function<void()> _noFunc) {
			popupName = _popupName;
			yesFunc = _yesFunc;
			noFunc = _noFunc;
		}

		void Show(std::string message) {
			label = message;
			show = true;
		}

		void SetShow(bool v) {
			show = v;
		}

		void Draw() {
			if (show)
			{
				ImGui::OpenPopup(popupName.c_str());
				show = false;
			}

			if (ImGui::BeginPopupModal(popupName.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text(label.c_str());
				ImGui::NewLine();
				//center
				float width = ImGui::GetContentRegionAvailWidth();
				ImVec2 cursorPos = ImGui::GetCursorScreenPos();
				ImGui::BeginGroup();
				{
					if (ImGui::Button("Yes", ImVec2(100.f, 25.f)))//YES
					{
						yesFunc();
					}
					ImGui::SameLine();
					if (ImGui::Button("No", ImVec2(100.f, 25.f)))//NO
					{
						noFunc();
					}
					ImGui::EndGroup();
				}
				ImGui::EndPopup();
			}
		}

	private:
		std::string popupName;
		std::string label;
		std::function<void()> yesFunc;
		std::function<void()> noFunc;
		bool show;
	};



	struct Table
	{
		struct Column
		{
			Column() {}
			Column(std::string _text, float _width) {
				text = _text;
				width = _width;
			}
			std::string text;
			float width;
		};


		Table() {}
		Table(std::vector<Column> _columns, float _totalWidth, float _cellHeight = 40.f, float _spaceBetweenColumns = 3.f, ImColor _headersBackgroungColor = ImColor(53, 114, 162, 140), ImColor _cellBackgroungColor = ImColor(53, 114, 162, 80)) {
			columns = _columns;
			spaceBetweenColumns = _spaceBetweenColumns;
			totalWidth = _totalWidth - ((_columns.size() - 1) * spaceBetweenColumns);
			cellHeight = _cellHeight;
			headersBackgroungColor = _headersBackgroungColor;
			cellBackgroungColor = _cellBackgroungColor;
		}

		void DrawColumnHeader(Column column, ImDrawList* draw_list, ImVec2 cursorPos, float FontSize = 13.f) {
			cursorPos = ImGui::GetCursorScreenPos();
			float width = totalWidth * column.width;
			ImVec2 RectFilledMax = ImVec2(cursorPos.x + width, cursorPos.y + cellHeight);
			draw_list->AddRectFilled(cursorPos, RectFilledMax, headersBackgroungColor);
			float TextSizeMultiplier = FontSize / 13.f;
			ImVec2 TextSize = ImGui::CalcTextSize(column.text.c_str());
			ImVec2 TextPos = ImVec2(cursorPos.x + ((width - (TextSize.x * TextSizeMultiplier)) * 0.5f), cursorPos.y + ((cellHeight - (TextSize.y * TextSizeMultiplier)) * 0.5f));
			ImFont* fontA = ImGui::GetDefaultFont();
			ImGui::BeginChild(std::string("TableHeader " + column.text).c_str(), ImVec2(width, cellHeight), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse); //ImGuiWindowFlags_HorizontalScrollbar
			ImGui::GetWindowDrawList()->AddText(fontA, FontSize, TextPos, ImColor(255, 255, 255, 255), column.text.c_str());
			ImGui::EndChild();
		}

		void BeginCell(float cellWidth, int cellID) {
			ImVec2 CursorPos = ImGui::GetCursorScreenPos();
			float width = totalWidth * cellWidth;

			ImGuiStyle& style = ImGui::GetStyle();
			style.ScrollbarSize = 8.f; // Set scrollbar size a bit more thin

			ImVec2 RectFilledMax = ImVec2(CursorPos.x + width, CursorPos.y + cellHeight);
			ImGui::GetWindowDrawList()->AddRectFilled(CursorPos, RectFilledMax, cellBackgroungColor);

			ImGui::PushID(cellID);
			ImGui::BeginChild(std::string("CellChild" + std::to_string(cellID)).c_str(), ImVec2(width, cellHeight), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse); //ImGuiWindowFlags_HorizontalScrollbar
		}

		void EndCell() {
			ImGui::EndChild();
			ImGui::PopID();
			ImGuiStyle& style = ImGui::GetStyle();
			style.ScrollbarSize = 16.f; // Set scrollbar size to default
		}

		void BeginRow(int rowID) {
			ImGui::BeginChild(std::string("##Row" + std::to_string(rowID)).c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), cellHeight));
		}

		void EndRow() {
			ImGui::EndChild();
		}

		void DrawEmptyCell(float cellWidth, int cellID) {
			BeginCell(cellWidth, cellID);
			EndCell();
		}
		
		void DrawEmptyRow(int rowID) {
			BeginRow(rowID);
			for (int n = 0; n < columns.size(); n++)
			{
				DrawEmptyCell(columns[n].width, n);
				if (n < columns.size() - 1)
					ImGui::SameLine(0.f, spaceBetweenColumns);
			}
			EndRow();
		}

		void DrawHeaders() {
			for (int n = 0; n < columns.size(); n++)
			{
				ImVec2 cursorPos = ImGui::GetCursorScreenPos();
				Column& column = columns[n];
				DrawColumnHeader(column, ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos());
				if (n < columns.size() - 1) //if not the last element
					ImGui::SameLine(0.f, spaceBetweenColumns);
			}
		}


		std::vector<Column> columns;
		float cellHeight;
		float totalWidth;
		float spaceBetweenColumns;
		ImColor headersBackgroungColor;
		ImColor cellBackgroungColor;
	};
}