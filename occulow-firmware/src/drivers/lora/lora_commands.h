/*
 * lora_commands.h
 *
 * Created: 4/18/2017 1:47:44 PM
 *  Author: tsun
 */


#ifndef LORA_COMMANDS_H_
#define LORA_COMMANDS_H_

static const char FACTORY_RESET_CMD[] = "sys factoryRESET\r\n";
static const char GET_VER[] = "sys get ver\r\n";
static const char SET_APPEUI_CMD[] = "mac set appeui %s\r\n";
static const char SET_DEVEUI_CMD[] = "mac set deveui %s\r\n";
static const char SET_APPKEY_CMD[] = "mac set appkey %s\r\n";
static const char SET_DATARATE_CMD[] = "mac set dr %s\r\n";
static const char SET_PWRIDX_CMD[] = "mac set pwridx %s\r\n";
static const char SET_ADR_CMD[] = "mac set adr %s\r\n";
static const char SET_DEVADDR_CMD[] = "mac set devaddr %s\r\n";
static const char SET_NWK_SKEY[] = "mac set nwkskey %s\r\n";
static const char SET_APP_SKEY[] = "mac set appskey %s\r\n";
static const char SET_PWR_CMD[] = "radio set pwr %s\r\n";
static const char SET_PRLEN_CMD[] = "radio set prlen %s\r\n";
static const char SAVE_CMD[] = "mac save\r\n";
static const char SET_CHANNEL_STATUS_CMD[] = "mac set ch status %d %s\r\n";
static const char JOIN_OTAA_CMD[] = "mac join otaa\r\n";
static const char JOIN_ABP_CMD[] = "mac join abp\r\n";
static const char SEND_UNCONF_CMD[] = "mac tx uncnf 1 %s\r\n";
static const char SLEEP_CMD[] = "sys sleep %d\r\n";
static const char AUTO_BAUD_CMD[] = {0x55, '\r', '\n'};

#endif /* LORA_COMMANDS_H_ */