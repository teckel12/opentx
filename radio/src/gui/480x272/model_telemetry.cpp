/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"

enum MenuModelTelemetryFrskyItems {
  ITEM_TELEMETRY_PROTOCOL_TYPE,
  ITEM_TELEMETRY_RSSI_LABEL,
  ITEM_TELEMETRY_RSSI_SOURCE,
  ITEM_TELEMETRY_RSSI_ALARM1,
  ITEM_TELEMETRY_RSSI_ALARM2,
  ITEM_TELEMETRY_DISABLE_ALARMS,
  ITEM_TELEMETRY_SENSORS_LABEL,
  ITEM_TELEMETRY_SENSOR_FIRST,
  ITEM_TELEMETRY_SENSOR_LAST = ITEM_TELEMETRY_SENSOR_FIRST + MAX_TELEMETRY_SENSORS - 1,
  ITEM_TELEMETRY_DISCOVER_SENSORS,
  ITEM_TELEMETRY_NEW_SENSOR,
  ITEM_TELEMETRY_DELETE_ALL_SENSORS,
  ITEM_TELEMETRY_IGNORE_SENSOR_INSTANCE,
#if defined(VARIO)
  ITEM_TELEMETRY_VARIO_LABEL,
  ITEM_TELEMETRY_VARIO_SOURCE,
  ITEM_TELEMETRY_VARIO_RANGE,
  ITEM_TELEMETRY_VARIO_CENTER,
#endif
  ITEM_TELEMETRY_MAX
};

#define TELEM_COL2                    200
#define TELEM_COL3                    275
#define TELEM_COL4                    350
#define TELEM_COL5                    425

#define RSSI_ROWS                     LABEL(RSSI), 0, 0, 0, 0,
#define SENSOR_ROWS(x)                (isTelemetryFieldAvailable(x) ? (uint8_t)0 : HIDDEN_ROW)
#define SENSORS_ROWS                  LABEL(Sensors), SENSOR_ROWS(0), SENSOR_ROWS(1), SENSOR_ROWS(2), SENSOR_ROWS(3), SENSOR_ROWS(4), SENSOR_ROWS(5), SENSOR_ROWS(6), SENSOR_ROWS(7), SENSOR_ROWS(8), SENSOR_ROWS(9), SENSOR_ROWS(10), SENSOR_ROWS(11), SENSOR_ROWS(12), SENSOR_ROWS(13), SENSOR_ROWS(14), SENSOR_ROWS(15), SENSOR_ROWS(16), SENSOR_ROWS(17), SENSOR_ROWS(18), SENSOR_ROWS(19), SENSOR_ROWS(20), SENSOR_ROWS(21), SENSOR_ROWS(22), SENSOR_ROWS(23), SENSOR_ROWS(24), SENSOR_ROWS(25), SENSOR_ROWS(26), SENSOR_ROWS(27), SENSOR_ROWS(28), SENSOR_ROWS(29), SENSOR_ROWS(30),  \
                                      SENSOR_ROWS(31), SENSOR_ROWS(32), SENSOR_ROWS(33), SENSOR_ROWS(34), SENSOR_ROWS(35), SENSOR_ROWS(36), SENSOR_ROWS(37), SENSOR_ROWS(38), SENSOR_ROWS(39), SENSOR_ROWS(40),SENSOR_ROWS(41), SENSOR_ROWS(42), SENSOR_ROWS(43), SENSOR_ROWS(44), SENSOR_ROWS(45), SENSOR_ROWS(46), SENSOR_ROWS(47), SENSOR_ROWS(48), SENSOR_ROWS(49), SENSOR_ROWS(50),SENSOR_ROWS(51), SENSOR_ROWS(52), SENSOR_ROWS(53), SENSOR_ROWS(54), SENSOR_ROWS(55), SENSOR_ROWS(56), SENSOR_ROWS(57), SENSOR_ROWS(58), SENSOR_ROWS(59), 0, 0, 0, 0,
#if defined(VARIO)
  #define VARIO_ROWS                  LABEL(Vario), 0, 1, 2,
#else
  #define VARIO_ROWS
#endif
#define TELEMETRY_TYPE_ROWS           (!IS_INTERNAL_MODULE_ENABLED() && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_PPM) ? (uint8_t)0 : HIDDEN_ROW,

void onSensorMenu(const char * result)
{
  uint8_t index = menuVerticalPosition - ITEM_TELEMETRY_SENSOR_FIRST;

  if (index < MAX_TELEMETRY_SENSORS) {
    if (result == STR_EDIT) {
      pushMenu(menuModelSensor);
    }
    else if (result == STR_DELETE) {
      delTelemetryIndex(index);
      index += 1;
      if (index<MAX_TELEMETRY_SENSORS && isTelemetryFieldAvailable(index))
        menuVerticalPosition += 1;
      else
        menuVerticalPosition = ITEM_TELEMETRY_NEW_SENSOR;
    }
    else if (result == STR_COPY) {
      int newIndex = availableTelemetryIndex();

      if (newIndex >= 0) {
        TelemetrySensor & sourceSensor = g_model.telemetrySensors[index];
        TelemetrySensor & newSensor = g_model.telemetrySensors[newIndex];
        newSensor = sourceSensor;
        TelemetryItem & sourceItem = telemetryItems[index];
        TelemetryItem & newItem = telemetryItems[newIndex];
        newItem = sourceItem;
        storageDirty(EE_MODEL);
      }
      else {
        POPUP_WARNING(STR_TELEMETRYFULL);
      }
    }
  }
}

void onDeleteAllSensorsConfirm(const char * result)
{
  if (result == STR_OK) {
    for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
      delTelemetryIndex(i);
    }
  }
}

bool menuModelTelemetry(event_t event)
{
  MENU(STR_MENUTELEMETRY, MODEL_ICONS, menuTabModel, MENU_MODEL_TELEMETRY, ITEM_TELEMETRY_MAX, { TELEMETRY_TYPE_ROWS RSSI_ROWS SENSORS_ROWS VARIO_ROWS });

  for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_CONTENT_TOP + i*FH;
    int k = i + menuVerticalOffset;
    for (int j=0; j<=k; j++) {
      if (mstate_tab[j] == HIDDEN_ROW)
        k++;
    }

    LcdFlags blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);
    LcdFlags attr = (menuVerticalPosition == k ? blink : 0);

    if (k >= ITEM_TELEMETRY_SENSOR_FIRST && k <= ITEM_TELEMETRY_SENSOR_LAST) {
      int index = k - ITEM_TELEMETRY_SENSOR_FIRST;
      lcdDrawNumber(MENUS_MARGIN_LEFT+INDENT_WIDTH, y, index+1, LEFT|attr, 0, NULL, ":");
      lcdDrawSizedText(60, y, g_model.telemetrySensors[index].label, TELEM_LABEL_LEN, ZCHAR);
      TelemetryItem & telemetryItem = telemetryItems[index];
      if (telemetryItem.isFresh()) {
        lcdDrawText(130, y, "*");
      }
      if (telemetryItem.isAvailable()) {
        LcdFlags color = telemetryItem.isOld() ? ALARM_COLOR : TEXT_COLOR;
        drawSensorCustomValue(TELEM_COL2, y, index, getValue(MIXSRC_FIRST_TELEM+3*index), LEFT|color);
      }
      else {
        lcdDrawText(TELEM_COL2, y, "---"); // TODO shortcut
      }
      TelemetrySensor * sensor = & g_model.telemetrySensors[index];
#if defined(MULTIMODULE)
      if (IS_SPEKTRUM_PROTOCOL()) {
          // Spektrum does not (yet?) really support multiple sensor of the same type. But a lot of
          // different sensor display the same information (e.g. voltage, capacity). Show the id
          // of the sensor in the overview to ease figuring out what sensors belong together
          lcdDrawHexNumber(TELEM_COL5, y, sensor->id, LEFT);
        } else
#endif
      if (sensor->type == TELEM_TYPE_CUSTOM && !g_model.ignoreSensorIds) {
        lcdDrawNumber(TELEM_COL5, y, sensor->instance, LEFT);
      }
      if (attr) {
        s_editMode = 0;
        s_currIdx = index;
        if (event == EVT_KEY_LONG(KEY_ENTER)) {
          killEvents(event);
          POPUP_MENU_ADD_ITEM(STR_EDIT);
          POPUP_MENU_ADD_ITEM(STR_COPY);
          POPUP_MENU_ADD_ITEM(STR_DELETE);
          POPUP_MENU_START(onSensorMenu);
        }
        else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
          pushMenu(menuModelSensor);
        }
      }
    }
    else

    switch (k) {
      case ITEM_TELEMETRY_PROTOCOL_TYPE:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_TELEMETRY_TYPE);
        lcdDrawTextAtIndex(TELEM_COL2, y, STR_TELEMETRY_PROTOCOLS, g_model.telemetryProtocol, attr);
        g_model.telemetryProtocol = checkIncDec(event, g_model.telemetryProtocol, PROTOCOL_TELEMETRY_FIRST, PROTOCOL_TELEMETRY_LAST, EE_MODEL, isTelemetryProtocolAvailable);
        break;

      case ITEM_TELEMETRY_SENSORS_LABEL:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_TELEMETRY_SENSORS);
        lcdDrawText(TELEM_COL2, y, STR_VALUE, 0);
        if (!g_model.ignoreSensorIds && !IS_SPEKTRUM_PROTOCOL()) {
          lcdDrawText(TELEM_COL5, y, STR_ID, 0);
        }
        break;

      case ITEM_TELEMETRY_DISCOVER_SENSORS:
        lcdDrawText(MENUS_MARGIN_LEFT+INDENT_WIDTH, y, allowNewSensors ? STR_STOP_DISCOVER_SENSORS : STR_DISCOVER_SENSORS, attr);
        if (attr && event==EVT_KEY_BREAK(KEY_ENTER)) {
          s_editMode = 0;
          allowNewSensors = !allowNewSensors;
        }
        break;

      case ITEM_TELEMETRY_NEW_SENSOR:
        lcdDrawText(MENUS_MARGIN_LEFT+INDENT_WIDTH, y, NO_INDENT(STR_TELEMETRY_NEWSENSOR), attr);
        if (attr && event==EVT_KEY_BREAK(KEY_ENTER)) {
          s_editMode = 0;
          int res = availableTelemetryIndex();
          if (res >= 0) {
            s_currIdx = res;
            pushMenu(menuModelSensor);
          }
          else {
            POPUP_WARNING(STR_TELEMETRYFULL);
          }
        }
        break;

      case ITEM_TELEMETRY_DELETE_ALL_SENSORS:
        lcdDrawText(MENUS_MARGIN_LEFT+INDENT_WIDTH, y, STR_DELETE_ALL_SENSORS, attr);
        if (attr)
          s_editMode = 0;
        if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
          killEvents(KEY_ENTER);
          POPUP_CONFIRMATION(STR_CONFIRMDELETE, onDeleteAllSensorsConfirm);
        }
        break;

      case ITEM_TELEMETRY_IGNORE_SENSOR_INSTANCE:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_IGNORE_INSTANCE);
        g_model.ignoreSensorIds = editCheckBox(g_model.ignoreSensorIds, TELEM_COL2, y, attr, event);
        break;

      case ITEM_TELEMETRY_RSSI_LABEL:
#if defined(MULTIMODULE)
        if (!IS_INTERNAL_MODULE_ON() &&
          g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_MULTIMODULE  &&
          g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false) == MODULE_SUBTYPE_MULTI_FS_AFHDS2A)
          lcdDrawText(MENUS_MARGIN_LEFT, y, "RSNR");
        else
#endif
        lcdDrawText(MENUS_MARGIN_LEFT, y, "RSSI");
        break;

      case ITEM_TELEMETRY_RSSI_SOURCE: {
        lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_SOURCE);
        if (g_model.rssiSource) {
          drawSource(TELEM_COL2, y, MIXSRC_FIRST_TELEM + 3 * (g_model.rssiSource - 1), attr);
          TelemetrySensor * sensor = &g_model.telemetrySensors[g_model.rssiSource - 1];
          lcdDrawText(lcdNextPos, y, " ", attr);
          drawReceiverName(lcdNextPos, y, sensor->frskyInstance.rxIndex >> 2, sensor->frskyInstance.rxIndex & 0x03, attr);
        }
        else {
          lcdDrawText(TELEM_COL2, y, STR_DEFAULT, attr);
        }
        if (attr) {
          g_model.rssiSource = checkIncDec(event, g_model.rssiSource, 0, MAX_TELEMETRY_SENSORS, EE_MODEL | NO_INCDEC_MARKS, isRssiSensorAvailable);
        }
        break;
      }

      case ITEM_TELEMETRY_RSSI_ALARM1:
      case ITEM_TELEMETRY_RSSI_ALARM2: {
        bool warning = (k==ITEM_TELEMETRY_RSSI_ALARM1);
        lcdDrawText(MENUS_MARGIN_LEFT, y, (warning ? STR_LOWALARM : STR_CRITICALALARM));
        lcdDrawNumber(TELEM_COL2, y, warning? g_model.rssiAlarms.getWarningRssi() : g_model.rssiAlarms.getCriticalRssi(), LEFT|attr, 3);

        if (attr && s_editMode>0) {
          if (warning)
            CHECK_INCDEC_MODELVAR(event, g_model.rssiAlarms.warning, -30, 30);
          else
            CHECK_INCDEC_MODELVAR(event, g_model.rssiAlarms.critical, -30, 30);
        }
        break;
      }
      case ITEM_TELEMETRY_DISABLE_ALARMS:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_DISABLE_ALARM);
        g_model.rssiAlarms.disabled = editCheckBox(g_model.rssiAlarms.disabled, TELEM_COL3, y, attr, event);
        break;
#if defined(VARIO)
      case ITEM_TELEMETRY_VARIO_LABEL:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_VARIO);
        break;

      case ITEM_TELEMETRY_VARIO_SOURCE:
        lcdDrawText(MENUS_MARGIN_LEFT, y, INDENT TR_SOURCE);
        drawSource(TELEM_COL2, y, g_model.varioData.source ? MIXSRC_FIRST_TELEM+3*(g_model.varioData.source-1) : 0, attr);
        if (attr) {
          g_model.varioData.source = checkIncDec(event, g_model.varioData.source, 0, MAX_TELEMETRY_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isSensorAvailable);
        }
        break;

      case ITEM_TELEMETRY_VARIO_RANGE:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_RANGE);
        lcdDrawNumber(TELEM_COL2, y, -10+g_model.varioData.min, (menuHorizontalPosition==0 ? attr : 0)|LEFT);
        lcdDrawNumber(TELEM_COL3, y, 10+g_model.varioData.max, (menuHorizontalPosition==1 ? attr : 0)|LEFT);
        if (attr && s_editMode>0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR(event, g_model.varioData.min, -7, 7);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.varioData.max, -7, 7);
              break;
          }
        }
        break;

      case ITEM_TELEMETRY_VARIO_CENTER:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_CENTER);
        lcdDrawNumber(TELEM_COL2, y, -5+g_model.varioData.centerMin, (menuHorizontalPosition==0 ? attr : 0)|PREC1|LEFT);
        lcdDrawNumber(TELEM_COL3, y, 5+g_model.varioData.centerMax, (menuHorizontalPosition==1 ? attr : 0)|PREC1|LEFT);
        lcdDrawTextAtIndex(TELEM_COL4, y, STR_VVARIOCENTER, g_model.varioData.centerSilent, (menuHorizontalPosition==2 ? attr : 0));
        if (attr && s_editMode>0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR(event, g_model.varioData.centerMin, -16, 5+min<int8_t>(10, g_model.varioData.centerMax+5));
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.varioData.centerMax, -5+max<int8_t>(-10, g_model.varioData.centerMin-5), +15);
              break;
            case 2:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.varioData.centerSilent, 1);
              break;
          }
        }
        break;
#endif
    }
  }
  return true;
}
