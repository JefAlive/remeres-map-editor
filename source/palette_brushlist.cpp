//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "palette_brushlist.h"
#include "gui.h"
#include "brush.h"
#include "sprites.h"
#include "add_tileset_window.h"
#include "add_item_window.h"
#include "materials.h"
#include "monsters.h"
#include "npcs.h"

// ============================================================================
// Brush Palette Panel
// A common class for terrain/doodad/item/raw palette

BEGIN_EVENT_TABLE(BrushPalettePanel, PalettePanel)
EVT_BUTTON(wxID_ADD, BrushPalettePanel::OnClickAddItemToTileset)
EVT_BUTTON(wxID_NEW, BrushPalettePanel::OnClickAddTileset)
EVT_BUTTON(wxID_FORWARD, BrushPalettePanel::OnNextPage)
EVT_BUTTON(wxID_BACKWARD, BrushPalettePanel::OnPreviousPage)
EVT_CHOICEBOOK_PAGE_CHANGING(wxID_ANY, BrushPalettePanel::OnSwitchingPage)
EVT_CHOICEBOOK_PAGE_CHANGED(wxID_ANY, BrushPalettePanel::OnPageChanged)
END_EVENT_TABLE()

BrushPalettePanel::BrushPalettePanel(wxWindow* parent, const TilesetContainer &tilesets, TilesetCategoryType category, wxWindowID id) :
	PalettePanel(parent, id),
	paletteType(category) {

	// Create the tileset panel
	const auto tsSizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Tileset");
	choicebook = newd wxChoicebook(this, wxID_ANY, wxDefaultPosition, wxSize(180, 250));
	tsSizer->Add(choicebook, 1, wxEXPAND);
	sizer->Add(tsSizer, 1, wxEXPAND);

	if (g_settings.getBoolean(Config::SHOW_TILESET_EDITOR)) {
		AddTilesetEditor();
	}

	sizer->Add(pageInfoSizer);

	for (auto it = tilesets.begin(); it != tilesets.end(); ++it) {
		const auto tilesetCategory = it->second->getCategory(category);
		if (tilesetCategory && !tilesetCategory->brushlist.empty()) {
			const auto panel = newd BrushPanel(choicebook, tilesetCategory);
			choicebook->AddPage(panel, wxstr(it->second->name));
		}
	}

	SetSizerAndFit(sizer);
}

BrushPalettePanel::~BrushPalettePanel() {
	if (currentPageCtrl) {
		currentPageCtrl->Unbind(wxEVT_SET_FOCUS, &BrushPalettePanel::OnSetFocus, this);
		currentPageCtrl->Unbind(wxEVT_KILL_FOCUS, &BrushPalettePanel::OnKillFocus, this);
		currentPageCtrl->Unbind(wxEVT_TEXT_ENTER, &BrushPalettePanel::OnSetPage, this);
	}
}

void BrushPalettePanel::OnSetFocus(wxFocusEvent &event) {
	g_gui.DisableHotkeys();
	event.Skip();
}

void BrushPalettePanel::OnKillFocus(wxFocusEvent &event) {
	g_gui.EnableHotkeys();
	event.Skip();
}

void BrushPalettePanel::RemovePagination() {
	pageInfoSizer->ShowItems(false);
	pageInfoSizer->Clear();

	if (nextPageButton) {
		nextPageButton->Destroy();
		nextPageButton = nullptr;
	}
	if (previousPageButton) {
		previousPageButton->Destroy();
		previousPageButton = nullptr;
	}
	if (currentPageCtrl) {
		currentPageCtrl->Destroy();
		currentPageCtrl = nullptr;
	}
	if (pageInfo) {
		pageInfo->Destroy();
		pageInfo = nullptr;
	}
}

void BrushPalettePanel::AddPagination() {
	RemovePagination();

	const auto buttonsSize = wxSize(55, 25);
	const auto middleElementsSize = wxSize(35, 25);

	nextPageButton = newd wxButton(this, wxID_FORWARD, "->", wxDefaultPosition, buttonsSize);
	currentPageCtrl = newd wxTextCtrl(this, wxID_ANY, "1", wxDefaultPosition, middleElementsSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
	pageInfo = newd wxStaticText(this, wxID_ANY, "/x", wxPoint(0, 5), middleElementsSize);
	previousPageButton = newd wxButton(this, wxID_BACKWARD, "<-", wxDefaultPosition, buttonsSize);

	currentPageCtrl->Bind(wxEVT_SET_FOCUS, &BrushPalettePanel::OnSetFocus, this);
	currentPageCtrl->Bind(wxEVT_KILL_FOCUS, &BrushPalettePanel::OnKillFocus, this);
	currentPageCtrl->Bind(wxEVT_TEXT_ENTER, &BrushPalettePanel::OnSetPage, this);

	pageInfoSizer->Add(previousPageButton, wxEXPAND);
	pageInfoSizer->AddSpacer(15);
	pageInfoSizer->Add(currentPageCtrl);
	pageInfoSizer->AddSpacer(5);
	pageInfoSizer->Add(pageInfo);
	pageInfoSizer->AddSpacer(15);
	pageInfoSizer->Add(nextPageButton, wxEXPAND);
}

void BrushPalettePanel::AddTilesetEditor() {
	const auto tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	const auto buttonAddTileset = newd wxButton(this, wxID_NEW, "Add new Tileset");
	tmpsizer->Add(buttonAddTileset, wxSizerFlags(0).Center());

	const auto buttonAddItemToTileset = newd wxButton(this, wxID_ADD, "Add new Item");
	tmpsizer->Add(buttonAddItemToTileset, wxSizerFlags(0).Center());

	sizer->Add(tmpsizer, 0, wxCENTER, 10);
}

void BrushPalettePanel::InvalidateContents() {
	for (auto pageIndex = 0; pageIndex < choicebook->GetPageCount(); ++pageIndex) {
		const auto panel = dynamic_cast<BrushPanel*>(choicebook->GetPage(pageIndex));
		panel->InvalidateContents();
	}
	PalettePanel::InvalidateContents();
}

void BrushPalettePanel::LoadCurrentContents() {
	const auto page = choicebook->GetCurrentPage();
	const auto panel = dynamic_cast<BrushPanel*>(page);
	if (panel) {
		panel->OnSwitchIn();
	}
	PalettePanel::LoadCurrentContents();
}

void BrushPalettePanel::LoadAllContents() {
	for (auto pageIndex = 0; pageIndex < choicebook->GetPageCount(); ++pageIndex) {
		const auto panel = dynamic_cast<BrushPanel*>(choicebook->GetPage(pageIndex));
		panel->LoadContents();
	}
	PalettePanel::LoadAllContents();
}

PaletteType BrushPalettePanel::GetType() const {
	return paletteType;
}

BrushListType BrushPalettePanel::GetListType() const {
	if (!choicebook) {
		return BRUSHLIST_LISTBOX;
	}

	const auto panel = dynamic_cast<BrushPanel*>(choicebook->GetPage(0));
	return panel->GetListType();
}

void BrushPalettePanel::SetListType(BrushListType newListType) {
	if (!choicebook) {
		return;
	}

	RemovePagination();

	// Large icons are shown in a single scrollable grid, so they don't need pagination.
	if (newListType == BRUSHLIST_SMALL_ICONS) {
		AddPagination();
	}

	for (auto pageIndex = 0; pageIndex < choicebook->GetPageCount(); ++pageIndex) {
		const auto panel = dynamic_cast<BrushPanel*>(choicebook->GetPage(pageIndex));
		panel->SetListType(newListType);
	}
}

void BrushPalettePanel::SetListType(const wxString &newListType) {
	if (!choicebook) {
		return;
	}

	const auto it = listTypeMap.find(newListType);
	if (it == listTypeMap.end()) {
		return;
	}

	const auto newListTypeEnum = (*it).second;

	SetListType(newListTypeEnum);
}

Brush* BrushPalettePanel::GetSelectedBrush() const {
	if (!choicebook) {
		return nullptr;
	}
	const auto page = choicebook->GetCurrentPage();
	const auto panel = dynamic_cast<BrushPanel*>(page);
	Brush* brush = nullptr;
	if (panel) {
		for (const auto &palettePanel : tool_bars) {
			brush = palettePanel->GetSelectedBrush();
			if (brush) {
				return brush;
			}
		}
		brush = panel->GetSelectedBrush();
	}
	return brush;
}

void BrushPalettePanel::SelectFirstBrush() {
	if (!choicebook) {
		return;
	}
	const auto page = choicebook->GetCurrentPage();
	const auto panel = dynamic_cast<BrushPanel*>(page);
	panel->SelectFirstBrush();
}

bool BrushPalettePanel::SelectBrush(const Brush* whatBrush) {
	if (!choicebook) {
		return false;
	}

	auto panel = dynamic_cast<BrushPanel*>(choicebook->GetCurrentPage());
	if (!panel) {
		return false;
	}

	if (panel->SelectBrush(whatBrush)) {
		for (const auto palettePanel : tool_bars) {
			palettePanel->SelectBrush(nullptr);
		}
		return true;
	}

	for (const auto palettePanel : tool_bars) {
		if (palettePanel->SelectBrush(whatBrush)) {
			panel->SelectBrush(nullptr);
			return true;
		}
	}

	for (auto pageIndex = 0; pageIndex < choicebook->GetPageCount(); ++pageIndex) {
		if (pageIndex == choicebook->GetSelection()) {
			continue;
		}

		panel = dynamic_cast<BrushPanel*>(choicebook->GetPage(pageIndex));
		if (panel && panel->SelectBrush(whatBrush)) {
			choicebook->ChangeSelection(pageIndex);
			for (const auto palettePanel : tool_bars) {
				palettePanel->SelectBrush(nullptr);
			}
			return true;
		}
	}
	return false;
}

void BrushPalettePanel::OnSwitchingPage(wxChoicebookEvent &event) {
	event.Skip();
	if (!choicebook) {
		return;
	}
	if (const auto oldPanel = dynamic_cast<BrushPanel*>(choicebook->GetCurrentPage()); oldPanel) {
		oldPanel->OnSwitchOut();
		for (const auto palettePanel : tool_bars) {
			const auto brush = palettePanel->GetSelectedBrush();
			if (brush) {
				rememberedBrushes[oldPanel] = brush;
			}
		}
	}

	const auto page = choicebook->GetPage(event.GetSelection());
	const auto panel = dynamic_cast<BrushPanel*>(page);
	if (panel) {
		panel->OnSwitchIn();
		const auto &brushbox = panel->GetBrushBox();
		const auto currentPage = brushbox->GetCurrentPage();
		const auto totalPages = brushbox->GetTotalPages();
		SetPageInfo(wxString::Format("/%d", totalPages));
		SetCurrentPage(wxString::Format("%d", currentPage));
		EnableNextPage(totalPages > currentPage);
		EnablePreviousPage(currentPage > 1);
		for (const auto palettePanel : tool_bars) {
			palettePanel->SelectBrush(rememberedBrushes[panel]);
		}
	}
}

void BrushPalettePanel::OnPageChanged(wxChoicebookEvent &event) {
	if (!choicebook) {
		return;
	}
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void BrushPalettePanel::OnSwitchIn() {
	LoadCurrentContents();
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SetBrushSizeInternal(last_brush_size);
	OnUpdateBrushSize(g_gui.GetBrushShape(), last_brush_size);
}

void BrushPalettePanel::OnClickAddTileset(wxCommandEvent &WXUNUSED(event)) {
	if (!choicebook) {
		return;
	}

	const auto window = newd AddTilesetWindow(g_gui.root, paletteType);
	const auto result = window->ShowModal();
	window->Destroy();

	if (result != 0) {
		g_gui.DestroyPalettes();
		g_gui.NewPalette();
	}
}

void BrushPalettePanel::OnClickAddItemToTileset(wxCommandEvent &WXUNUSED(event)) {
	if (!choicebook) {
		return;
	}
	const auto &tilesetName = choicebook->GetPageText(choicebook->GetSelection()).ToStdString();

	const auto it = g_materials.tilesets.find(tilesetName);

	if (it != g_materials.tilesets.end()) {
		const auto window = newd AddItemWindow(g_gui.root, paletteType, it->second);
		const auto result = window->ShowModal();
		window->Destroy();

		if (result != 0) {
			g_gui.RebuildPalettes();
		}
	}
}

void BrushPalettePanel::OnPageUpdate(BrushBoxInterface* brushbox, int page) {
	if (brushbox->SetPage(page)) {
		const auto currentPage = brushbox->GetCurrentPage();
		const auto totalPages = brushbox->GetTotalPages();
		currentPageCtrl->SetValue(wxString::Format("%d", currentPage));
		Fit();
		g_gui.aui_manager->Update();
		brushbox->SelectFirstBrush();
		nextPageButton->Enable(totalPages > currentPage);
		previousPageButton->Enable(currentPage > 1);
	}
}

void BrushPalettePanel::OnSetPage(wxCommandEvent &WXUNUSED(event)) {
	const auto &brushPanel = dynamic_cast<BrushPanel*>(choicebook->GetCurrentPage());
	if (!brushPanel) {
		return;
	}

	const auto &brushbox = brushPanel->GetBrushBox();

	int page;
	if (!currentPageCtrl->GetValue().ToInt(&page)) {
		return;
	}

	if (page > brushbox->GetTotalPages() || page < 1) {
		return;
	}

	OnPageUpdate(brushbox, page);
}

void BrushPalettePanel::OnNextPage(wxCommandEvent &WXUNUSED(event)) {
	const auto &brushPanel = dynamic_cast<BrushPanel*>(choicebook->GetCurrentPage());
	if (brushPanel) {
		const auto &brushbox = brushPanel->GetBrushBox();
		OnPageUpdate(brushbox, brushbox->GetCurrentPage() + 1);
	}
}
void BrushPalettePanel::OnPreviousPage(wxCommandEvent &WXUNUSED(event)) {
	const auto &brushPanel = dynamic_cast<BrushPanel*>(choicebook->GetCurrentPage());
	if (brushPanel) {
		const auto &brushbox = brushPanel->GetBrushBox();
		OnPageUpdate(brushbox, brushbox->GetCurrentPage() - 1);
	}
}

void BrushPalettePanel::EnableNextPage(bool enable /* = true*/) {
	if (!nextPageButton) {
		return;
	}
	nextPageButton->Enable(enable);
}

void BrushPalettePanel::EnablePreviousPage(bool enable /* = true*/) {
	if (!previousPageButton) {
		return;
	}
	previousPageButton->Enable(enable);
}

void BrushPalettePanel::SetPageInfo(const wxString &text) {
	if (!pageInfo) {
		return;
	}
	pageInfo->SetLabelText(text);
}

void BrushPalettePanel::SetCurrentPage(const wxString &value) {
	if (!currentPageCtrl) {
		return;
	}
	currentPageCtrl->SetValue(value);
}

// ============================================================================
// Brush Panel
// A container of brush buttons

BEGIN_EVENT_TABLE(BrushPanel, wxPanel)
// Listbox style
EVT_LISTBOX(wxID_ANY, BrushPanel::OnClickListBoxRow)
END_EVENT_TABLE()

BrushPanel::BrushPanel(wxWindow* parent, const TilesetCategory* tileset) :
	wxPanel(parent, wxID_ANY), tileset(tileset) {
	SetSizerAndFit(sizer);
}

void BrushPanel::AssignTileset(const TilesetCategory* newTileset) {
	if (newTileset != tileset) {
		InvalidateContents();
		tileset = newTileset;
	}
}

BrushListType BrushPanel::GetListType() const {
	return listType;
}

void BrushPanel::SetListType(BrushListType newListType) {
	if (listType != newListType) {
		InvalidateContents();
		listType = newListType;
	}
}

void BrushPanel::SetListType(const wxString &newListType) {
	const auto it = listTypeMap.find(newListType);
	if (it != listTypeMap.end()) {
		SetListType(it->second);
	}
}

void BrushPanel::InvalidateContents() {
	sizer->Clear(true);
	loaded = false;
	brushbox = nullptr;
}

void BrushPanel::LoadContents() {
	if (loaded) {
		return;
	}
	loaded = true;
	ASSERT(tileset != nullptr);
	switch (listType) {
		case BRUSHLIST_LARGE_ICONS:
			brushbox = newd BrushIconBox(this, tileset, RENDER_SIZE_48x48, true);
			break;
		case BRUSHLIST_SMALL_ICONS:
			brushbox = newd BrushIconBox(this, tileset, RENDER_SIZE_16x16);
			break;
		case BRUSHLIST_LISTBOX:
			brushbox = newd BrushListBox(this, tileset);
			break;
		default:
			break;
	}
	ASSERT(brushbox != nullptr);
	sizer->Add(brushbox->GetSelfWindow(), 1, wxEXPAND);
	Fit();
	brushbox->SelectFirstBrush();
}

void BrushPanel::SelectFirstBrush() {
	if (loaded) {
		ASSERT(brushbox != nullptr);
		brushbox->SelectFirstBrush();
	}
}

Brush* BrushPanel::GetSelectedBrush() const {
	if (loaded) {
		ASSERT(brushbox != nullptr);
		return brushbox->GetSelectedBrush();
	}

	if (tileset && tileset->size() > 0) {
		return tileset->brushlist[0];
	}
	return nullptr;
}

bool BrushPanel::SelectBrush(const Brush* whatBrush) {
	if (loaded) {
		// std::cout << loaded << std::endl;
		// std::cout << brushbox << std::endl;
		ASSERT(brushbox != nullptr);
		return brushbox->SelectBrush(whatBrush);
	}

	for (const auto brush : tileset->brushlist) {
		if (brush == whatBrush) {
			LoadContents();
			return brushbox->SelectBrush(whatBrush);
		}
	}
	return false;
}

void BrushPanel::OnSwitchIn() {
	LoadContents();
}

void BrushPanel::OnSwitchOut() {
	////
}

void BrushPanel::OnClickListBoxRow(wxCommandEvent &event) {
	ASSERT(tileset->getType() >= TILESET_UNKNOWN && tileset->getType() <= TILESET_HOUSE);
	// We just notify the GUI of the action, it will take care of everything else
	ASSERT(brushbox);
	const auto index = event.GetSelection();

	if (const auto paletteWindow = g_gui.GetParentWindowByType<PaletteWindow*>(this); paletteWindow != nullptr) {
		g_gui.ActivatePalette(paletteWindow);
	}

	g_gui.SelectBrush(tileset->brushlist[index], tileset->getType());
}

BrushBoxInterface* BrushPanel::GetBrushBox() const {
	return brushbox;
}

// ============================================================================
// BrushIconBox

BEGIN_EVENT_TABLE(BrushIconBox, wxScrolledWindow)
// Listbox style
EVT_TOGGLEBUTTON(wxID_ANY, BrushIconBox::OnClickBrushButton)
EVT_SIZE(BrushIconBox::OnSize)
// Virtual grid style (custom painted, no child windows)
EVT_PAINT(BrushIconBox::OnPaint)
EVT_LEFT_DOWN(BrushIconBox::OnLeftDown)
EVT_MOTION(BrushIconBox::OnMotion)
EVT_KEY_DOWN(BrushIconBox::OnKey)
END_EVENT_TABLE()

BrushIconBox::BrushIconBox(wxWindow* parent, const TilesetCategory* tileset, RenderSize rsz, bool scrollEnabled) :
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL),
	BrushBoxInterface(tileset),
	iconSize(rsz),
	scrollable(scrollEnabled) {
	ASSERT(tileset->getType() >= TILESET_UNKNOWN && tileset->getType() <= TILESET_HOUSE);

	if (scrollable) {
		// Large icons are shown in a single custom-painted grid. We intentionally
		// do not create one window per brush: the grid only renders the rows that
		// are actually visible (plus a small overscan), which keeps scrolling
		// smooth even for tilesets with thousands of items.
		totalPages = 1;
		SetBackgroundStyle(wxBG_STYLE_PAINT);
		SetScrollRate(0, GetIconExtent());
		RecalculateVirtualSize();
		return;
	}

	width = iconSize == RENDER_SIZE_32x32 ? std::max(g_settings.getInteger(Config::PALETTE_COL_COUNT) / 2 + 1, 1) : std::max(g_settings.getInteger(Config::PALETTE_COL_COUNT) + 1, 1);
	height = iconSize == RENDER_SIZE_32x32 ? std::max(g_settings.getInteger(Config::PALETTE_ROW_COUNT) / 2 + 1, 1) : std::max(g_settings.getInteger(Config::PALETTE_ROW_COUNT) + 1, 1);

	const auto totalItems = (width * height);
	totalPages = (tileset->brushlist.size() / totalItems) + 1;

	SetScrollbars(20, 20, 8, 0, 0, 0, false);

	brushButtons.reserve(totalItems);

	LoadContentByPage();

	const auto &brushPalettePanel = g_gui.GetParentWindowByType<BrushPalettePanel*>(this);
	if (brushPalettePanel) {
		brushPalettePanel->SetPageInfo(wxString::Format("/%d", totalPages));
		brushPalettePanel->EnableNextPage(totalPages > currentPage);
		brushPalettePanel->EnablePreviousPage(currentPage > 1);
	}
}

bool BrushIconBox::LoadContentByPage(int page /* = 1 */) {
	if (page <= 0 || page > totalPages) {
		return false;
	}

	currentPage = page;

	const auto startOffset = (width * height) * (page - 1);
	auto endOffset = (width * height) * page;
	endOffset = page > 1 ? endOffset : startOffset + endOffset;
	endOffset = endOffset > tileset->brushlist.size() ? tileset->brushlist.size() : endOffset;

	if (stacksizer) {
		stacksizer->ShowItems(false);
		stacksizer->Clear();
		rowsizers.clear();
		brushButtons.clear();
	}

	stacksizer = newd wxBoxSizer(wxVERTICAL);
	SetSizer(stacksizer);

	auto rowSizer = newd wxBoxSizer(wxHORIZONTAL);

	for (auto i = startOffset; i < endOffset; ++i) {
		const auto brushButton = newd BrushButton(this, tileset->brushlist[i], iconSize);
		brushButtons.emplace_back(brushButton);
		rowSizer->Add(brushButton);

		if (brushButtons.size() % width == 0) {
			stacksizer->Add(rowSizer);
			rowsizers.emplace_back(rowSizer);
			rowSizer = newd wxBoxSizer(wxHORIZONTAL);
		}
	}

	if (rowsizers.size() <= 0 || rowSizer != rowsizers.back()) {
		stacksizer->Add(rowSizer);
		rowsizers.emplace_back(rowSizer);
	}

	if (!stacksizer->AreAnyItemsShown()) {
		stacksizer->ShowItems(true);
	}

	return true;
}

bool BrushIconBox::LoadAllContents() {
	if (!scrollable) {
		return false;
	}

	// The virtual grid has no per-item windows to (re)create; it only needs its
	// geometry refreshed so the scrollbar range matches the tileset size.
	selectedIndex = -1;
	RecalculateVirtualSize();
	Refresh();
	return true;
}

int BrushIconBox::ComputeColumns() const {
	const auto clientWidth = GetClientSize().GetWidth();
	return std::max(clientWidth / GetIconExtent(), 1);
}

int BrushIconBox::GetIconExtent() const {
	switch (iconSize) {
		case RENDER_SIZE_16x16:
			return 20;
		case RENDER_SIZE_48x48:
			return 52;
		case RENDER_SIZE_32x32:
		default:
			return 36;
	}
}

wxSize BrushIconBox::DoGetBestClientSize() const {
	if (scrollable) {
		return wxSize(180, 240);
	}
	return wxScrolledWindow::DoGetBestClientSize();
}

void BrushIconBox::OnSize(wxSizeEvent &event) {
	event.Skip();

	if (!scrollable) {
		return;
	}

	RecalculateVirtualSize();
	Refresh();
}

void BrushIconBox::SelectFirstBrush() {
	if (!tileset || tileset->size() == 0) {
		return;
	}

	if (scrollable) {
		SelectIndex(0);
		return;
	}

	Select(brushButtons[0]);
}

Brush* BrushIconBox::GetSelectedBrush() const {
	if (!tileset) {
		return nullptr;
	}

	if (scrollable) {
		if (selectedIndex >= 0 && selectedIndex < static_cast<int>(tileset->brushlist.size())) {
			return tileset->brushlist[selectedIndex];
		}
		return nullptr;
	}

	return selectedButton ? selectedButton->brush : nullptr;
}

bool BrushIconBox::SelectPaginatedBrush(const Brush* whatBrush, BrushPalettePanel* brushPalettePanel) {
	if (scrollable) {
		if (!whatBrush) {
			SelectIndex(-1);
			return false;
		}

		const auto it = std::ranges::find(tileset->brushlist, whatBrush);
		if (it == tileset->brushlist.end()) {
			return false;
		}

		SelectIndex(static_cast<int>(std::distance(tileset->brushlist.begin(), it)));
		return true;
	}

	const auto brushIt = std::ranges::find(tileset->brushlist.begin(), tileset->brushlist.end(), whatBrush);

	if (brushIt != tileset->brushlist.end()) {
		const auto index = static_cast<size_t>(std::distance(tileset->brushlist.begin(), brushIt));
		const auto pageSize = static_cast<size_t>(width * height);
		const auto page = static_cast<int>(index / pageSize) + 1;
		if (currentPage != page) {
			brushPalettePanel->OnPageUpdate(this, page);
		}

		const auto it = std::ranges::find_if(brushButtons, [&](const auto &brushButton) {
			return brushButton->brush == whatBrush;
		});

		if (it != brushButtons.end()) {
			Select(*it);
			return true;
		}

		return false;
	}

	return false;
}

bool BrushIconBox::SelectBrush(const Brush* whatBrush) {
	if (scrollable) {
		if (!whatBrush) {
			SelectIndex(-1);
			return false;
		}

		const auto it = std::ranges::find(tileset->brushlist, whatBrush);
		if (it == tileset->brushlist.end()) {
			SelectIndex(-1);
			return false;
		}

		SelectIndex(static_cast<int>(std::distance(tileset->brushlist.begin(), it)));
		return true;
	}

	Deselect();

	if (!whatBrush) {
		return false;
	}

	const auto &brushPalettePanel = g_gui.GetParentWindowByType<BrushPalettePanel*>(GetSelfWindow());
	const auto listType = brushPalettePanel->GetListType();
	if (listType == BRUSHLIST_LARGE_ICONS || listType == BRUSHLIST_SMALL_ICONS) {
		return SelectPaginatedBrush(whatBrush, brushPalettePanel);
	}

	const auto it = std::ranges::find_if(brushButtons, [&](const auto &brushButton) {
		return brushButton->brush == whatBrush;
	});

	if (it != brushButtons.end()) {
		Select(*it);
		return true;
	}

	return false;
}

bool BrushIconBox::NextPage() {
	return !scrollable && LoadContentByPage(currentPage + 1);
}

bool BrushIconBox::SetPage(int page) {
	return !scrollable && LoadContentByPage(page);
}

bool BrushIconBox::PreviousPage() {
	return !scrollable && LoadContentByPage(currentPage - 1);
}

void BrushIconBox::Select(BrushButton* brushButton) {
	if (scrollable || !brushButton) {
		return;
	}

	Deselect();
	selectedButton = brushButton;
	selectedButton->SetValue(true);
	EnsureVisible(selectedButton);
}

void BrushIconBox::Deselect() {
	if (selectedButton != nullptr) {
		selectedButton->SetValue(false);
		selectedButton = nullptr;
	}
}

void BrushIconBox::EnsureVisible(const BrushButton* whatBrush) {
	if (scrollable || whatBrush == nullptr) {
		return;
	}

	int windowSizeX, windowSizeY;
	GetVirtualSize(&windowSizeX, &windowSizeY);

	int scrollUnitX;
	int scrollUnitY;
	GetScrollPixelsPerUnit(&scrollUnitX, &scrollUnitY);

	const auto &rect = whatBrush->GetRect();
	int y;
	CalcUnscrolledPosition(0, rect.y, nullptr, &y);

	const auto maxScrollPos = windowSizeY / scrollUnitY;
	const auto scrollPosY = std::min(maxScrollPos, (y / scrollUnitY));

	int startScrollPosY;
	GetViewStart(nullptr, &startScrollPosY);

	int clientSizeX, clientSizeY;
	GetClientSize(&clientSizeX, &clientSizeY);
	const auto endScrollPosY = startScrollPosY + clientSizeY / scrollUnitY;

	if (scrollPosY < startScrollPosY || scrollPosY > endScrollPosY) {
		// only scroll if the button isnt visible
		Scroll(-1, scrollPosY);
	}
}

void BrushIconBox::SelectIndex(int index) {
	if (!tileset) {
		return;
	}

	if (index < 0 || index >= static_cast<int>(tileset->brushlist.size())) {
		if (selectedIndex != -1) {
			selectedIndex = -1;
			Refresh();
		}
		return;
	}

	selectedIndex = index;
	EnsureIndexVisible(index);
	Refresh();
}

void BrushIconBox::EnsureIndexVisible(int index) {
	if (!scrollable || columns <= 0 || !tileset) {
		return;
	}

	const int extent = std::max(GetIconExtent(), 1);
	const int row = index / columns;

	int startRow = 0;
	GetViewStart(nullptr, &startRow);

	const wxSize client = GetClientSize();
	const int visibleRows = std::max((client.GetHeight() + extent - 1) / extent, 1);

	if (row < startRow) {
		Scroll(-1, row);
	} else if (row >= startRow + visibleRows) {
		Scroll(-1, row - visibleRows + 1);
	}
}

void BrushIconBox::RecalculateVirtualSize() {
	if (!scrollable || !tileset) {
		return;
	}

	const int extent = std::max(GetIconExtent(), 1);
	columns = std::max(ComputeColumns(), 1);

	const int total = static_cast<int>(tileset->brushlist.size());
	const int rows = (total + columns - 1) / columns;
	const wxSize client = GetClientSize();

	SetVirtualSize(
		std::max(client.GetWidth(), columns * extent),
		std::max(client.GetHeight(), rows * extent)
	);
}

int BrushIconBox::HitTestIndex(const wxPoint &clientPos) const {
	if (!scrollable || !tileset || columns <= 0) {
		return -1;
	}

	int x = clientPos.x;
	int y = clientPos.y;
	CalcUnscrolledPosition(x, y, &x, &y);

	if (x < 0 || y < 0) {
		return -1;
	}

	const int extent = std::max(GetIconExtent(), 1);
	const int col = x / extent;
	const int row = y / extent;
	if (col < 0 || col >= columns) {
		return -1;
	}

	const int index = row * columns + col;
	if (index < 0 || index >= static_cast<int>(tileset->brushlist.size())) {
		return -1;
	}

	return index;
}

void BrushIconBox::DrawBrushTile(wxDC &dc, int index, int x, int y) const {
	if (!tileset || index < 0 || index >= static_cast<int>(tileset->brushlist.size())) {
		return;
	}

	const int extent = std::max(GetIconExtent(), 1);
	const bool selected = (index == selectedIndex);

	DCButton::DrawButtonFrame(dc, x, y, extent, extent, selected);

	const auto sprite = g_gui.gfx.getSprite(tileset->brushlist[index]->getLookID());
	if (!sprite) {
		return;
	}

	switch (iconSize) {
		case RENDER_SIZE_16x16:
			sprite->DrawTo(&dc, SPRITE_SIZE_16x16, x + 2, y + 2);
			break;
		case RENDER_SIZE_32x32:
			sprite->DrawTo(&dc, SPRITE_SIZE_32x32, x + 2, y + 2);
			break;
		case RENDER_SIZE_48x48:
			sprite->DrawTo(&dc, SPRITE_SIZE_48x48, x + 2, y + 2, 48, 48);
			break;
	}

	if (selected && g_settings.getInteger(Config::USE_GUI_SELECTION_SHADOW)) {
		if (const auto marker = g_gui.gfx.getSprite(EDITOR_SPRITE_SELECTION_MARKER)) {
			marker->DrawTo(&dc, SPRITE_SIZE_32x32, x + 2, y + 2, extent - 4, extent - 4);
		}
	}
}

void BrushIconBox::OnPaint(wxPaintEvent &event) {
	if (!scrollable) {
		// Paginated mode uses real child buttons; let wxScrolledWindow paint.
		event.Skip();
		return;
	}

	wxAutoBufferedPaintDC dc(this);
	DoPrepareDC(dc);

	dc.SetBackground(wxBrush(GetBackgroundColour()));
	dc.Clear();

	if (!tileset || tileset->brushlist.empty()) {
		return;
	}

	const int extent = std::max(GetIconExtent(), 1);
	const int cols = std::max(columns, 1);
	const int total = static_cast<int>(tileset->brushlist.size());
	const int totalRows = (total + cols - 1) / cols;

	int viewStartX = 0;
	int viewStartY = 0;
	GetViewStart(&viewStartX, &viewStartY);

	const wxSize client = GetClientSize();
	const int visibleRows = (client.GetHeight() + extent - 1) / extent;

	// Virtual window: only render the visible rows plus an overscan buffer so
	// that scrolling never exposes unrendered (white) rows.
	const int firstRow = std::max(0, viewStartY - OVERSCAN_ROWS);
	const int lastRow = std::min(totalRows - 1, viewStartY + visibleRows + OVERSCAN_ROWS);

	for (int row = firstRow; row <= lastRow; ++row) {
		const int rowY = row * extent;
		for (int col = 0; col < cols; ++col) {
			const int index = row * cols + col;
			if (index >= total) {
				break;
			}
			DrawBrushTile(dc, index, col * extent, rowY);
		}
	}
}

void BrushIconBox::OnLeftDown(wxMouseEvent &event) {
	if (!scrollable) {
		event.Skip();
		return;
	}

	const int index = HitTestIndex(event.GetPosition());
	if (index >= 0) {
		SelectIndex(index);
		if (const auto paletteWindow = g_gui.GetParentWindowByType<PaletteWindow*>(this); paletteWindow) {
			g_gui.ActivatePalette(paletteWindow);
		}
		g_gui.SelectBrush(tileset->brushlist[index], tileset->getType());
	}

	SetFocus();
}

void BrushIconBox::OnMotion(wxMouseEvent &event) {
	if (!scrollable) {
		event.Skip();
		return;
	}

	const int index = HitTestIndex(event.GetPosition());
	if (index != hoveredIndex) {
		hoveredIndex = index;
		SetToolTip(index >= 0 ? wxstr(tileset->brushlist[index]->getName()) : wxString());
	}

	event.Skip();
}

void BrushIconBox::OnKey(wxKeyEvent &event) {
	if (scrollable) {
		g_gui.AddPendingCanvasEvent(event);
		return;
	}

	event.Skip();
}

void BrushIconBox::OnClickBrushButton(wxCommandEvent &event) {
	const auto eventObject = event.GetEventObject();
	const auto brushButton = dynamic_cast<BrushButton*>(eventObject);
	if (brushButton) {
		if (const auto paletteWindow = g_gui.GetParentWindowByType<PaletteWindow*>(this); paletteWindow) {
			g_gui.ActivatePalette(paletteWindow);
		}
		g_gui.SelectBrush(brushButton->brush, tileset->getType());
	}
}

// ============================================================================
// BrushListBox

BEGIN_EVENT_TABLE(BrushListBox, wxVListBox)
EVT_KEY_DOWN(BrushListBox::OnKey)
END_EVENT_TABLE()

BrushListBox::BrushListBox(wxWindow* parent, const TilesetCategory* tileset) :
	wxVListBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE),
	BrushBoxInterface(tileset) {
	SetItemCount(tileset->size());
}

void BrushListBox::SelectFirstBrush() {
	SetSelection(0);
	wxWindow::ScrollLines(-1);
}

Brush* BrushListBox::GetSelectedBrush() const {
	if (!tileset) {
		return nullptr;
	}

	if (const auto index = GetSelection(); index != wxNOT_FOUND) {
		return tileset->brushlist[index];
	} else if (tileset->size() > 0) {
		return tileset->brushlist[0];
	}
	return nullptr;
}

bool BrushListBox::SelectPaginatedBrush(const Brush* whatBrush, BrushPalettePanel* brushPalettePanel) noexcept {
	return false;
}

bool BrushListBox::SelectBrush(const Brush* whatBrush) {
	for (auto index = 0; index < tileset->brushlist.size(); ++index) {
		if (tileset->brushlist[index] == whatBrush) {
			SetSelection(index);
			return true;
		}
	}
	return false;
}

void BrushListBox::OnDrawItem(wxDC &dc, const wxRect &rect, size_t index) const {
	ASSERT(index < tileset->size());
	if (const auto sprite = g_gui.gfx.getSprite(tileset->brushlist[index]->getLookID()); sprite) {
		sprite->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
	} else {
		auto monsterType = g_monsters[tileset->brushlist[index]->getName()];
		NpcType* npcType = nullptr;
		if (!monsterType) {
			npcType = g_npcs[tileset->brushlist[index]->getName()];
		}
		int lookType = 0;
		if (monsterType) {
			lookType = monsterType->outfit.lookType;
		} else if (npcType) {
			lookType = npcType->outfit.lookType;
		}
		if (lookType == 0) {
			lookType = 197; // This looktype is a tribute to our beloved Carl-bot from OpenTibiaBR Discord.
		}
		auto creatureSprite = g_gui.gfx.getCreatureSprite(lookType);
		if (creatureSprite) {
			creatureSprite->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
		}
	}
	if (IsSelected(index)) {
		if (HasFocus()) {
			dc.SetTextForeground(wxColor(0xFF, 0xFF, 0xFF));
		} else {
			dc.SetTextForeground(wxColor(0x00, 0x00, 0xFF));
		}
	} else {
		dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
	}
	dc.DrawText(wxstr(tileset->brushlist[index]->getName()), rect.GetX() + 40, rect.GetY() + 6);
}

wxCoord BrushListBox::OnMeasureItem(size_t index) const {
	return 32;
}

void BrushListBox::OnKey(wxKeyEvent &event) {
	switch (event.GetKeyCode()) {
		case WXK_UP:
		case WXK_DOWN:
		case WXK_LEFT:
		case WXK_RIGHT:
			if (g_settings.getInteger(Config::LISTBOX_EATS_ALL_EVENTS)) {
				case WXK_PAGEUP:
				case WXK_PAGEDOWN:
				case WXK_HOME:
				case WXK_END:
					event.Skip(true);
			} else {
				[[fallthrough]];
				default:
					if (g_gui.GetCurrentTab() != nullptr) {
						g_gui.GetCurrentMapTab()->GetEventHandler()->AddPendingEvent(event);
					}
			}
	}
}
