#include <StdAfx.h>
#include "PhatSDK.h"
#include "PlayerModule.h"
#include "WeenieFactory.h"

DEFINE_PACK(ShortCutData)
{
	pWriter->Write<uint32_t>(index_);
	pWriter->Write<uint32_t>(objectID_);
	pWriter->Write<uint32_t>(spellID_);
}

DEFINE_UNPACK(ShortCutData)
{
	index_ = pReader->Read<uint32_t>();
	objectID_ = pReader->Read<uint32_t>();
	spellID_ = pReader->Read<uint32_t>();
	return true;
}

const uint32_t SHORTCUTMANAGER_MAX_SHORTCUTS = 18;

ShortCutManager::~ShortCutManager()
{
	Destroy();
}

void ShortCutManager::Destroy()
{
	if (shortCuts_)
	{
		for (uint32_t i = 0; i < SHORTCUTMANAGER_MAX_SHORTCUTS; i++)
		{
			SafeDelete (shortCuts_[i]);
		}

		delete [] shortCuts_;
		shortCuts_ = NULL;
	}
}

ShortCutManager &ShortCutManager::operator =(const ShortCutManager &other)
{
	if (&other == this)
		return *this;

	Destroy();

	if (other.shortCuts_)
	{
		shortCuts_ = new ShortCutData *[SHORTCUTMANAGER_MAX_SHORTCUTS];
		memset(shortCuts_, 0, sizeof(ShortCutData *) * SHORTCUTMANAGER_MAX_SHORTCUTS);

		for (uint32_t i = 0; i < SHORTCUTMANAGER_MAX_SHORTCUTS; i++)
		{
			if (other.shortCuts_[i])
			{
				ShortCutData *dataCopy = new ShortCutData;
				*dataCopy = *other.shortCuts_[i];
				shortCuts_[i] = dataCopy;
			}
		}
	}

	return *this;
}

void ShortCutManager::AddShortCut(const ShortCutData &data)
{
	if (data.index_ < 0 || data.index_ >= SHORTCUTMANAGER_MAX_SHORTCUTS)
		return;

	if (!shortCuts_)
	{
		shortCuts_ = new ShortCutData *[SHORTCUTMANAGER_MAX_SHORTCUTS];
		memset(shortCuts_, 0, sizeof(ShortCutData *) * SHORTCUTMANAGER_MAX_SHORTCUTS);
	}

	ShortCutData *shortcutEntry = shortCuts_[data.index_];
	if (!shortcutEntry)
	{
		shortcutEntry = new ShortCutData;
		shortCuts_[data.index_] = shortcutEntry;
	}

	*shortcutEntry = data;
}

void ShortCutManager::RemoveShortCut(int index)
{
	if (!shortCuts_)
		return;

	if (index < 0 || index >= SHORTCUTMANAGER_MAX_SHORTCUTS)
		return;

	SafeDelete(shortCuts_[index]);
}

DEFINE_PACK(ShortCutManager)
{
	uint32_t numEntries = 0;

	if (shortCuts_)
	{
		for (uint32_t i = 0; i < SHORTCUTMANAGER_MAX_SHORTCUTS; i++)
		{
			if (shortCuts_[i])
				numEntries++;
		}
	}

	pWriter->Write<uint32_t>(numEntries);

	if (shortCuts_)
	{
		for (uint32_t i = 0; i < SHORTCUTMANAGER_MAX_SHORTCUTS; i++)
		{
			if (shortCuts_[i])
				shortCuts_[i]->Pack(pWriter);
		}
	}
}

DEFINE_UNPACK(ShortCutManager)
{
	Destroy();

	uint32_t numEntries = pReader->Read<uint32_t>();

	shortCuts_ = new ShortCutData *[SHORTCUTMANAGER_MAX_SHORTCUTS];
	memset(shortCuts_, 0, sizeof(ShortCutData *) * SHORTCUTMANAGER_MAX_SHORTCUTS);

	if (numEntries > SHORTCUTMANAGER_MAX_SHORTCUTS)
		numEntries = SHORTCUTMANAGER_MAX_SHORTCUTS;

	for (uint32_t i = 0; i < numEntries; i++)
	{
		ShortCutData data;
		data.UnPack(pReader);

		if (data.index_ >= 0 && data.index_ < SHORTCUTMANAGER_MAX_SHORTCUTS)
		{
			ShortCutData *entry = shortCuts_[data.index_];
			if (!entry)
			{
				entry = new ShortCutData;
				shortCuts_[data.index_] = entry;
			}

			*entry = data;
		}
	}

	return true;
}

GenericQualitiesData::~GenericQualitiesData()
{
	SafeDelete(m_pIntStatsTable);
	SafeDelete(m_pBoolStatsTable);
	SafeDelete(m_pFloatStatsTable);
	SafeDelete(m_pStrStatsTable);
}

BOOL GenericQualitiesData::InqString(uint32_t key, std::string &value)
{
	if (m_pStrStatsTable)
	{
		const std::string *pValue = m_pStrStatsTable->lookup(key);

		if (pValue)
		{
			value = *pValue;
			return TRUE;
		}
	}

	return FALSE;
}

DEFINE_PACK(GenericQualitiesData)
{
	uint32_t header = 0;
	if (m_pIntStatsTable)
		header |= 1;
	if (m_pBoolStatsTable)
		header |= 2;
	if (m_pFloatStatsTable)
		header |= 4;
	if (m_pStrStatsTable)
		header |= 8;

	pWriter->Write<uint32_t>(header);

	if (m_pIntStatsTable)
		m_pIntStatsTable->Pack(pWriter);
	if (m_pBoolStatsTable)
		m_pBoolStatsTable->Pack(pWriter);
	if (m_pFloatStatsTable)
		m_pFloatStatsTable->Pack(pWriter);
	if (m_pStrStatsTable)
		m_pStrStatsTable->Pack(pWriter);
}

DEFINE_UNPACK(GenericQualitiesData)
{
	uint32_t header = pReader->Read<uint32_t>();
	if (header & 1)
	{
		if (!m_pIntStatsTable)
			m_pIntStatsTable = new PackableHashTable<uint32_t, int32_t>();

		m_pIntStatsTable->UnPack(pReader);
	}
	if (header & 2)
	{
		if (!m_pBoolStatsTable)
			m_pBoolStatsTable = new PackableHashTable<uint32_t, int32_t>();

		m_pBoolStatsTable->UnPack(pReader);
	}
	if (header & 4)
	{
		if (!m_pFloatStatsTable)
			m_pFloatStatsTable = new PackableHashTable<uint32_t, double>();

		m_pFloatStatsTable->UnPack(pReader);
	}
	if (header & 8)
	{
		if (!m_pStrStatsTable)
			m_pStrStatsTable = new PackableHashTable<uint32_t, std::string>();

		m_pStrStatsTable->UnPack(pReader);
	}

	return true;
}

DEFINE_PACK(BaseProperty)
{
	
}

DEFINE_UNPACK(BaseProperty)
{
	return true;
}

BasePropertyValue *BaseProperty::CreatePropertyValue(uint32_t propName)
{
	UNFINISHED();
	return NULL;
#if 0
	switch (propName)
	{
	case PropertyNameEnum::UICore_Menu_default_selected_item_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Menu_listbox_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Menu_open_center_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Menu_open_down_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Menu_open_up_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Menu_popup_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Menu_popup_layout_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UICore_Menu_selection_display_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Menu_text_item_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Menu_text_item_layout_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UICore_Button_boolean_button_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Button_broadcast_when_ghosted_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Button_ghosted_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Button_highlighted_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Button_hot_button_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Button_hot_click_first_interval_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Button_hot_click_interval_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Button_click_action_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Button_rollover_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Text_horizontal_justification_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Text_vertical_justification_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Text_editable_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Text_entry_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Text_font_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UICore_Text_font_color_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::UICore_Text_fonts_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UICore_Text_font_colors_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UICore_Text_tag_fonts_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UICore_Text_tag_font_colors_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UICore_Text_max_chars_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Text_no_IME_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Text_one_line_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Text_outline_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Text_font_outline_color_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::UICore_Text_left_margin_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Text_right_margin_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Text_top_margin_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Text_bottom_margin_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Text_selectable_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Text_trim_from_top_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Text_fit_to_text_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Resizebar_border_bottom_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Resizebar_border_left_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Resizebar_border_right_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Resizebar_border_top_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Panel_pages_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UICore_Panel_page_data_PropertyName: return new StructPropertyValue;
	case PropertyNameEnum::UICore_Panel_tab_element_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Panel_page_element_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Panel_page_open_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_activatable_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_activate_on_show_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_focus_on_show_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_container_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_context_menu_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_disallow_drag_in_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_disallow_drag_out_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_dragable_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_hide_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_max_height_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Element_max_width_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Element_min_height_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Element_min_width_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Element_blocks_clicks_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_notify_on_resize_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_notify_on_move_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_notify_on_create_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_resize_line_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_save_loc_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_save_size_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_tooltip_ID_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Element_tooltip_layout_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UICore_Element_tooltip_entry_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Element_tooltip_text_ID_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Element_tooltips_on_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_viewport_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_alphablendmod_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Element_InputMapID_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Element_HoverDelay_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Element_ShouldEraseBackground_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_ClampGameViewEdgeToObject_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Element_DrawAfterChildren_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_ImageTileOffset_X_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Element_ImageTileOffset_Y_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Element_ImageTileOffset_PropertyName: return new StructPropertyValue;
	case PropertyNameEnum::UICore_Element_VisibilityToggle_InputAction_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Element_VisibilityToggle_ToggleType_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_ListBox_click_select_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_ListBox_drag_rollover_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_ListBox_drag_select_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_ListBox_horizontal_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_ListBox_item_normal_state_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_ListBox_item_selected_state_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_ListBox_max_columns_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_ListBox_max_rows_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_ListBox_selected_item_state_change_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_ListBox_entry_template_element_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_ListBox_entry_template_layout_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UICore_ListBox_entry_templates_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UICore_ListBox_entry_template_PropertyName: return new StructPropertyValue;
	case PropertyNameEnum::UICore_Meter_goal_position_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Meter_frame_meter_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Meter_move_fill_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Meter_position_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Meter_smooth_movement_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Meter_smooth_movement_duration_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Meter_frame_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UICore_Meter_frame_array_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UICore_Meter_frame_array_draw_mode_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Meter_child_direction_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Field_drag_rollover_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollable_horizontal_scrollbar_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Scrollable_vertical_scrollbar_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Scrollable_keep_in_bounds_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollable_preserve_percentage_on_resize_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_disallow_updating_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_disabled_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_increment_button_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_decrement_button_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_hide_when_disabled_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_ghost_when_disabled_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_horizontal_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_move_to_touched_position_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_num_stop_locations_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_stops_dont_include_endpoints_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_stop_locations_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_page_first_interval_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_page_interval_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_proportional_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_smooth_movement_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_smooth_movement_duration_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_goal_position_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_position_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_current_stop_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_widget_size_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Scrollbar_min_widget_size_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_ContextMenu_open_delay_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_ContextMenu_lvl_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_ContextMenu_index_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Dialog_topmost_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Dialog_type_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Dialog_confirmation_yesbuttontext_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Dialog_confirmation_nobuttontext_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Dialog_confirmation_response_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Dialog_message_okbuttontext_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Dialog_textinput_donebuttontext_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Dialog_textinput_response_PropertyName: return new StringPropertyValue;
	case PropertyNameEnum::UICore_Dialog_confirmationtextinput_donebuttontext_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Dialog_confirmationtextinput_cancelbuttontext_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Dialog_confirmationtextinput_response_PropertyName: return new StringPropertyValue;
	case PropertyNameEnum::UICore_Dialog_wait_key_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Dialog_menu_item_array_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UICore_Dialog_menu_item_text_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Dialog_menu_okbuttontext_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Dialog_menu_choice_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Dialog_confirmationmenu_item_array_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UICore_Dialog_confirmationmenu_item_text_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Dialog_confirmationmenu_okbuttontext_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Dialog_confirmationmenu_cancelbuttontext_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Dialog_confirmationmenu_choice_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Dialog_modal_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Browser_url_PropertyName: return new StringPropertyValue;
	case PropertyNameEnum::UICore_ColorPicker_show_selection_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_ColorPicker_current_selection_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::UICore_GroupBox_default_button_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_GroupBox_selected_button_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_GroupBox_allow_button_click_on_selected_button_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Dialog_queue_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Dialog_text_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Dialog_duration_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UICore_Text_truncate_text_to_fit_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UICore_Text_lose_focus_on_escape_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Text_lose_focus_on_acceptinput_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Element_ObjectMode_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UICore_Text_auto_tooltip_truncated_text_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UICore_Text_select_all_on_gainfocus_mousedown_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::GameplayOptionList_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::GameplayOption_PropertyName: return new StructPropertyValue;
	case PropertyNameEnum::Option_Name_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::Option_Description_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::Option_PropertyName_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::Option_MaxFloatValue_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::Option_MinFloatValue_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::Option_MaxIntValue_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::Option_MinIntValue_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::Option_NumStops_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::VisibilityLayer_PropertyName: return new StringPropertyValue;
	case PropertyNameEnum::WB_RenderMaterial_Grid_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::Tools_VisibilityArray_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::Tools_VisibilityStruct_PropertyName: return new StructPropertyValue;
	case PropertyNameEnum::Tools_Visibility_Properties_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::Tools_VisibilityInheritance_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::Tools_VisibilityState_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::SaveOnExit_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::WorldBuilderSounds_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::DefaultTextEditor_PropertyName: return new StringPropertyValue;
	case PropertyNameEnum::DefaultTexture_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::LandblockAutosaveTimer_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::LandblockAutosaveRCSCheckEnabled_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ShowEmptyEditors_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::AlwaysStartInEntityWorkspace_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::RandomHeadingRotation_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::FlushRCP_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::CopyScreenshotToClipboard_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ShareFilePropertyLastDirectory_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ShowEncounters_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ShowEntityLinks_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::ShowEntityHeading_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ShowMayaStyleUI_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ShowEntityOrigin_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::EntityPasteOffset_PropertyName: return new VectorPropertyValue;
	case PropertyNameEnum::EntityMovementOffset_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::EntityNudgeOffset_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::MoveRelativeToXYZ_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UndoStackSize_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::EntityCullingDistance_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::OverrideNudgeCommand_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UsePhysicsPlacement_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UseSnapPoints_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::BackgroundColor_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::AmbientLightColor_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::UseAmbientLight_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ForceSunlight_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::DefaultRegion_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::LandblockDrawingRadius_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::LandblockOutlineColor_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::ShowLandblockOutline_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::CellOutlineColor_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::TeleportOffset_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UseLandblockCache_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ShowLandscapeInDungeon_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ShowEnvCells_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ContentOrGeometry_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::GridEnabled_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::GridSnap_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::GridAngleSnap_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::GridSnapAngle_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::GridRange_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::GridBlocks_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::GridCells_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::GridXYAxisColor_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::GridMajorAxisColor_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::GridMinorAxisColor_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::CameraFlySpeed_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::CameraMoveSpeed_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::CameraRotateSpeed_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::InvertMouseX_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::InvertMouseY_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::InvertStrafe_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::CursorPlacementMode_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::CursorColor_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::SelectionColor_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::SelectionBlinks_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ShowPortalSelection_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::PortalSelectionColor_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::PortalSelectionBlinks_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::LockBlocksInPerforce_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ToggleTerrainRaycast_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::ToggleCeilingRaycast_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::VisualizeCeiling_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::Physics_EtherealityType_PropertyName: return new Bitfield32PropertyValue;
	case PropertyNameEnum::Physics_EtherealToType_PropertyName: return new Bitfield32PropertyValue;
	case PropertyNameEnum::Physics_PlacementEtherealToType_PropertyName: return new Bitfield32PropertyValue;
	case PropertyNameEnum::WB_Camera_Entity_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::WB_LoadError_Entity_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::WB_LightSource_Entity_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::ShowSnapPoints_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::SpawnSecondEditor_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::Physics_AdjustableScale_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::LinkName_PropertyName: return new StringPropertyValue;
	case PropertyNameEnum::LinkColor_PropertyName: return new ColorPropertyValue;
	case PropertyNameEnum::ShowAppliedPropertyType_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::ShowEnvCellsInWorld_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::RefreshLandscape_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_Allegiance_VassalID_PropertyName: return new InstanceIDPropertyValue;
	case PropertyNameEnum::UI_Credits_TextArea_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Credits_StringTable_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UI_Credits_Duration_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UI_Credits_PictureArray_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UI_Credits_SoundArray_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UI_Credits_Sound_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UI_Credits_Picture_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UI_CharacterManagement_CharacterInstanceID_PropertyName: return new InstanceIDPropertyValue;
	case PropertyNameEnum::UI_Chargen_SkillID_PropertyName: return new InstanceIDPropertyValue;
	case PropertyNameEnum::UI_Chat_TalkFocus_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Effects_EffectsUIType_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Fellowship_FellowID_PropertyName: return new InstanceIDPropertyValue;
	case PropertyNameEnum::UI_ItemList_ItemSlotID_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_ItemList_ItemID_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_ItemList_SpellID_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_ItemList_IsContainer_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_ItemList_IsShortcut_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_ItemList_IsVendor_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_ItemList_IsSalvage_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_ItemList_FixedListSize_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_ItemList_AllowDragging_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_ItemList_AtLeastOneEmptySlot_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_Keyboard_ActionMappingListBox_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Keyboard_OKButton_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Keyboard_CancelButton_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Keyboard_ResetToDefaultsButton_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Keyboard_RevertToSavedButton_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Keyboard_CurrentKeymapLabel_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Keyboard_LoadKeymapButton_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Keyboard_SaveKeymapAsButton_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Keyboard_KeymapFilename_PropertyName: return new StringPropertyValue;
	case PropertyNameEnum::UI_MiniGame_PieceIconArray_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UI_MiniGame_PieceIcon_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UI_Options_Revert_Title_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UI_Options_Default_Title_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UI_Options_Menu_Value_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Options_Menu_FontIndex_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Options_Menu_ColorIndex_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_SpewBox_MaxMessagesToDisplay_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UICore_Element_PanelID_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_ActionKeyMap_ResetToDefaultsButton_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_ActionKeyMap_BindButtons_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UI_ActionKeyMap_BindButton_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Radar_Radius_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Radar_CenterPoint_PropertyName: return new StructPropertyValue;
	case PropertyNameEnum::UI_Radar_X_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Radar_Y_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Radar_NorthToken_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Radar_SouthToken_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Radar_EastToken_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Radar_WestToken_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Radar_CoordinatesContainer_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Radar_CombinedCoordsField_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Radar_XCoordField_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Radar_YCoordField_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Vendor_ShopFilters_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_InfoRegion_Index_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_InfoRegion_StatType_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_InfoRegion_StatID_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Usage_ToolID_PropertyName: return new InstanceIDPropertyValue;
	case PropertyNameEnum::UI_Usage_TargetID_PropertyName: return new InstanceIDPropertyValue;
	case PropertyNameEnum::UI_Spellbook_DeletedSpellID_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_StatManagement_SkillToTrainID_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_StatManagement_CostToTrain_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_ItemList_ShortcutOverlayArray_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UI_ItemList_ShortcutOverlayArray_Ghosted_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UI_ItemList_ShortcutOverlay_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UI_Intro_StateArray_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UI_Intro_State_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Panel_RestoreLastOpenPanelWhenClosed_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_SpellComponent_ComponentDataID_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UI_SpellComponentHeader_ComponentCategory_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Map_MapX0_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Map_MapX1_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Map_MapY0_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Map_MapY1_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_ItemList_SingleSelection_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_ItemList_DragScroll_Horizontal_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_ItemList_DragScroll_Vertical_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_ItemList_DragScroll_JumpDistance_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_ItemList_DragScroll_MarginWidth_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_ItemList_DragScroll_MarginHeight_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_ItemList_DragScroll_Delay_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UI_ItemList_DragScroll_ItemScrolling_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_ItemList_DragScroll_SpellScrolling_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::UI_ItemList_ShortcutOverlayArray_Empty_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::UI_Admin_QualityType_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Admin_QualityEnum_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Admin_NewEntry_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Admin_ListBoxEntryType_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Chat_WindowID_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::Option_TextType_PropertyName: return new Bitfield64PropertyValue;
	case PropertyNameEnum::Option_DefaultOpacity_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::Option_ActiveOpacity_PropertyName: return new FloatPropertyValue;
	case PropertyNameEnum::UIOption_CheckboxBitfield_FullyCheckedImage_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UIOption_CheckboxBitfield_PartlyCheckedImage_PropertyName: return new DataFilePropertyValue;
	case PropertyNameEnum::UIOption_CheckboxBitfield_ChildIndex_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::UI_Social_FriendID_PropertyName: return new InstanceIDPropertyValue;
	case PropertyNameEnum::Option_Placement_X_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::Option_Placement_Y_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::Option_Placement_Width_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::Option_Placement_Height_PropertyName: return new IntegerPropertyValue;
	case PropertyNameEnum::Option_Placement_Visibility_PropertyName: return new BoolPropertyValue;
	case PropertyNameEnum::Option_Placement_PropertyName: return new StructPropertyValue;
	case PropertyNameEnum::Option_PlacementArray_PropertyName: return new ArrayPropertyValue;
	case PropertyNameEnum::Option_Placement_Title_PropertyName: return new StringInfoPropertyValue;
	case PropertyNameEnum::UI_Social_CharacterTitleID_PropertyName: return new EnumPropertyValue;
	case PropertyNameEnum::UI_Social_SquelchID_PropertyName: return new InstanceIDPropertyValue;
	}

	return NULL;
#endif
}

DEFINE_PACK(PackObjPropertyCollection)
{
	pWriter->Write<uint32_t>(2);
	m_hashProperties.Pack(pWriter);
}

DEFINE_UNPACK(PackObjPropertyCollection)
{
	pReader->Read<uint32_t>();
	m_hashProperties.UnPack(pReader);
	return true;
}

PlayerModule::~PlayerModule()
{
	Destroy();
}

void PlayerModule::Destroy()
{
	SafeDelete(shortcuts_);
	SafeDelete(desired_comps_);
	SafeDelete(m_pPlayerOptionsData);
	if(windowDataLength)
		SafeDeleteArray(windowData);
}

PlayerModule &PlayerModule::operator=(const PlayerModule &other)
{
	if (&other)
	{
		options_ = other.options_;
		options2_ = other.options2_;
		spell_filters_ = other.spell_filters_;

		for (uint32_t i = 0; i < 8; i++)
			favorite_spells_[i] = other.favorite_spells_[i];

		CloneMemberPointerData<ShortCutManager>(shortcuts_, other.shortcuts_);
		CloneMemberPointerData<PackableHashTable<uint32_t, int32_t>>(desired_comps_, other.desired_comps_);
		CloneMemberPointerData<GenericQualitiesData>(m_pPlayerOptionsData, other.m_pPlayerOptionsData);
	}

	return *this;
}

void PlayerModule::SetPackHeader(uint32_t *bitfield)
{
	if (shortcuts_)
		*bitfield |= 1u;

	*bitfield |= 0x400u;

	if (desired_comps_)
		*bitfield |= 8;

	*bitfield |= 0x60;

	if (m_pPlayerOptionsData)
		*bitfield |= 0x100;

	if (windowDataLength > 0)
		*bitfield |= 0x200u;
}

DEFINE_PACK(PlayerModule)
{
	uint32_t header = 0;
	SetPackHeader(&header);

	pWriter->Write<uint32_t>(header);
	pWriter->Write<uint32_t>(options_);

	if (shortcuts_)
		shortcuts_->Pack(pWriter);

	for (uint32_t i = 0; i < 8; i++)
		favorite_spells_[i].Pack(pWriter);

	if (desired_comps_)
		desired_comps_->Pack(pWriter);

	pWriter->Write<uint32_t>(spell_filters_);
	pWriter->Write<uint32_t>(options2_);
	
	if (m_pPlayerOptionsData)
		m_pPlayerOptionsData->Pack(pWriter);

	if (windowDataLength > 0)
	{
		pWriter->Write(windowData, windowDataLength);
	}
	
	pWriter->Align();
}

DEFINE_UNPACK(PlayerModule)
{
	uint32_t header = pReader->Read<uint32_t>();
	options_ = pReader->Read<uint32_t>();

	if (header & 1)
	{
		if (!shortcuts_)
			shortcuts_ = new ShortCutManager;

		shortcuts_->UnPack(pReader);
	}
	else
	{
		SafeDelete(shortcuts_);
	}

	favorite_spells_[0].UnPack(pReader);

	if (header & 4)
	{
		for (uint32_t i = 1; i < 5; i++)
			favorite_spells_[i].UnPack(pReader);
	}
	else if (header & 0x10)
	{
		for (uint32_t i = 1; i < 7; i++)
			favorite_spells_[i].UnPack(pReader);
	}
	else if (header & 0x400)
	{
		for (uint32_t i = 1; i < 8; i++)
			favorite_spells_[i].UnPack(pReader);
	}

	if (header & 8)
	{
		if (!desired_comps_)
			desired_comps_ = new PackableHashTable<uint32_t, int32_t>();

		desired_comps_->UnPack(pReader);
	}
	else
	{
		SafeDelete(desired_comps_);
	}

	if (header & 0x20)
	{
		spell_filters_ = pReader->Read<uint32_t>();
	}
	else
	{
		spell_filters_ = 0x3FFF;
	}

	if (header & 0x40)
	{
		options2_ = pReader->Read<uint32_t>();
	}
	else
	{
		options2_ = 0x948700;
	}

	if (header & 0x80)
	{
		m_TimeStampFormat = pReader->ReadString();
	}

	if (header & 0x100)
	{
		if (!m_pPlayerOptionsData)
			m_pPlayerOptionsData = new GenericQualitiesData();

		m_pPlayerOptionsData->UnPack(pReader);
		m_pPlayerOptionsData->InqString(1, m_TimeStampFormat);
	}
	else
	{
		SafeDelete(m_pPlayerOptionsData);
	}

	if (header & 0x200)
	{
		windowDataLength = pReader->GetDataRemaining();
		windowData = new BYTE[windowDataLength];
		memcpy(windowData, pReader->ReadArray(windowDataLength), windowDataLength);
	}

	pReader->ReadAlign();

	return true;
}

void PlayerModule::AddShortCut(ShortCutData &data)
{
	if (!shortcuts_)
		shortcuts_ = new ShortCutManager;

	shortcuts_->AddShortCut(data);
}

void PlayerModule::RemoveShortCut(int index)
{
	if (!shortcuts_)
		return;

	shortcuts_->RemoveShortCut(index);
}

void PlayerModule::AddSpellFavorite(uint32_t spell_id, int index, int spellBarIndex)
{
	if (spellBarIndex < 0 || spellBarIndex >= 8)
		return;

	RemoveSpellFavorite(spell_id, spellBarIndex);
	favorite_spells_[spellBarIndex].InsertAt(index, spell_id);
}

void PlayerModule::RemoveSpellFavorite(uint32_t spell_id, int spellBarIndex)
{
	if (spellBarIndex < 0 || spellBarIndex >= 8)
		return;
	
	PackableList<uint32_t> *spellBar = &favorite_spells_[spellBarIndex];

	for (PackableList<uint32_t>::iterator i = spellBar->begin(); i != spellBar->end();)
	{
		if (*i == spell_id)
			i = spellBar->erase(i);
		else
			i++;
	}
}

void PlayerModule::AddOrUpdateDesiredComp(uint32_t & compWCID, uint32_t & compQTY)
{
	if (compQTY == 0) { // remove desired comp - do this first to save time.
		if (desired_comps_ && desired_comps_->exists(compWCID)) {
			desired_comps_->erase(compWCID);
		}
		return;
	}

	CWeenieDefaults *defaults = g_pWeenieFactory->GetWeenieDefaults(compWCID); //pull defaults for requested wcid
	if (!defaults)
		return;
	if (defaults->m_Qualities.GetInt(ITEM_TYPE_INT, 0) != 0x1000) { // ignore WCIDS without an item_type of 0x1000 (Spell Component)
		return;
	}
	if (!desired_comps_) { // this should never happen with "current" playerModule init code.
		desired_comps_ = new PackableHashTable<uint32_t, int32_t>();
	}

	if (compQTY < 5001) { //client only allows input of quantities up to 5000.
		desired_comps_->insert_or_assign(compWCID, compQTY);
	}
}
