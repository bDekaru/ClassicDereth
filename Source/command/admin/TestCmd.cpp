#include <StdAfx.h>
#include "easylogging++.h"
#include "Client.h"
#include "WeenieObject.h"
#include "Monster.h"
#include "Player.h"
#include "World.h"
#include "Config.h"
#include "Globals.h"
#include "Server.h"
#include "WeenieFactory.h"

#include "ClientCommands.h"

CLIENT_COMMAND(testchat, "", "Sends hello back in all chat channel formats.", ADMIN_ACCESS, SERVER_CATEGORY)
{
	pPlayer->SendText("Sending text via LTT_DEFAULT", LTT_DEFAULT);
	pPlayer->SendText("Sending text via LTT_ALL_CHANNELS", LTT_ALL_CHANNELS);
	pPlayer->SendText("Sending text via LTT_SPEECH", LTT_SPEECH);
	pPlayer->SendText("Sending text via LTT_SPEECH_DIRECT", LTT_SPEECH_DIRECT);
	pPlayer->SendText("Sending text via LTT_SPEECH_DIRECT_SEND", LTT_SPEECH_DIRECT_SEND);
	pPlayer->SendText("Sending text via LTT_SYSTEM_EVENT", LTT_SYSTEM_EVENT);
	pPlayer->SendText("Sending text via LTT_COMBAT", LTT_COMBAT);
	pPlayer->SendText("Sending text via LTT_MAGIC", LTT_MAGIC);
	pPlayer->SendText("Sending text via LTT_CHANNEL", LTT_CHANNEL);
	pPlayer->SendText("Sending text via LTT_CHANNEL_SEND", LTT_CHANNEL_SEND);
	pPlayer->SendText("Sending text via LTT_SOCIAL_CHANNEL", LTT_SOCIAL_CHANNEL);
	pPlayer->SendText("Sending text via LTT_SOCIAL_CHANNEL_SEND", LTT_SOCIAL_CHANNEL_SEND);
	pPlayer->SendText("Sending text via LTT_EMOTE", LTT_EMOTE);
	pPlayer->SendText("Sending text via LTT_ADVANCEMENT", LTT_ADVANCEMENT);
	pPlayer->SendText("Sending text via LTT_ABUSE_CHANNEL", LTT_ABUSE_CHANNEL);
	pPlayer->SendText("Sending text via LTT_HELP_CHANNEL", LTT_HELP_CHANNEL);
	pPlayer->SendText("Sending text via LTT_APPRAISAL_CHANNEL", LTT_APPRAISAL_CHANNEL);
	pPlayer->SendText("Sending text via LTT_MAGIC_CASTING_CHANNEL", LTT_MAGIC_CASTING_CHANNEL);
	pPlayer->SendText("Sending text via LTT_ALLEGIENCE_CHANNEL", LTT_ALLEGIENCE_CHANNEL);
	pPlayer->SendText("Sending text via LTT_FELLOWSHIP_CHANNEL", LTT_FELLOWSHIP_CHANNEL);
	pPlayer->SendText("Sending text via LTT_WORLD_BROADCAST", LTT_WORLD_BROADCAST);
	pPlayer->SendText("Sending text via LTT_COMBAT_ENEMY", LTT_COMBAT_ENEMY);
	pPlayer->SendText("Sending text via LTT_COMBAT_SELF", LTT_COMBAT_SELF);
	pPlayer->SendText("Sending text via LTT_RECALL", LTT_RECALL);
	pPlayer->SendText("Sending text via LTT_CRAFT", LTT_CRAFT);
	pPlayer->SendText("Sending text via LTT_SALVAGING", LTT_SALVAGING);
	pPlayer->SendText("Sending text via LTT_ERROR", LTT_ERROR);
	pPlayer->SendText("Sending text via LTT_GENERAL_CHANNEL", LTT_GENERAL_CHANNEL);
	pPlayer->SendText("Sending text via LTT_TRADE_CHANNEL", LTT_TRADE_CHANNEL);
	pPlayer->SendText("Sending text via LTT_LFG_CHANNEL", LTT_LFG_CHANNEL);
	pPlayer->SendText("Sending text via LTT_ROLEPLAY_CHANNEL", LTT_ROLEPLAY_CHANNEL);
	pPlayer->SendText("Sending text via LTT_SPEECH_DIRECT_ADMIN", LTT_SPEECH_DIRECT_ADMIN);
	pPlayer->SendText("Sending text via LTT_SOCIETY_CHANNEL", LTT_SOCIETY_CHANNEL);
	pPlayer->SendText("Sending text via LTT_OLTHOI_CHANNEL", LTT_OLTHOI_CHANNEL);

	return false;
}

CLIENT_COMMAND(wcidbodyall, "wcid", "Sets wcid defaults for wcid to all body locations with 10 dam. Skips breath of DID30 not found.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	if (argc < 1)
	{
		return true;
	}

	int wcid = atoi(argv[0]);
	if (!wcid)
		return true;

	CWeenieDefaults* def = g_pWeenieFactory->GetWeenieDefaults(wcid);

	BodyPart* head = new BodyPart();
	head->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	head->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	head->_dval = 0.0;
	head->_dvar = .5;

	BodyPart* chest = new BodyPart();
	chest->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	chest->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	chest->_dval = 1.0;
	chest->_dvar = .5;

	BodyPart* abdomen = new BodyPart();
	abdomen->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	abdomen->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	abdomen->_dval = 2.0;
	abdomen->_dvar = .5;

	BodyPart* upperarm = new BodyPart();
	upperarm->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	upperarm->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	upperarm->_dval = 3.0;
	upperarm->_dvar = .5;

	BodyPart* lowerarm = new BodyPart();
	lowerarm->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	lowerarm->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	lowerarm->_dval = 4.0;
	lowerarm->_dvar = .5;

	BodyPart* hand = new BodyPart();
	hand->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	hand->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	hand->_dval = 5.0;
	hand->_dvar = .5;

	BodyPart* upperleg = new BodyPart();
	upperleg->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	upperleg->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	upperleg->_dval = 6.0;
	upperleg->_dvar = .5;

	BodyPart* lowerleg = new BodyPart();
	lowerleg->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	lowerleg->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	lowerleg->_dval = 7.0;
	lowerleg->_dvar = .5;

	BodyPart* foot = new BodyPart();
	foot->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	foot->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	foot->_dval = 8.0;
	foot->_dvar = .5;

	BodyPart* horn = new BodyPart();
	horn->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	horn->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	horn->_dval = 9;
	horn->_dvar = .5;

	BodyPart* frontleg = new BodyPart();
	frontleg->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	frontleg->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	frontleg->_dval = 10.0;
	frontleg->_dvar = .5;

	BodyPart* frontfoot = new BodyPart();
	frontfoot->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	frontfoot->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	frontfoot->_dval = 12.0;
	frontfoot->_dvar = .5;

	BodyPart* rearleg = new BodyPart();
	rearleg->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	rearleg->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	rearleg->_dval = 13;
	rearleg->_dvar = .5;

	BodyPart* rearfoot = new BodyPart();
	rearfoot->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	rearfoot->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	rearfoot->_dval = 15;
	rearfoot->_dvar = .5;

	BodyPart* torso = new BodyPart();
	torso->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	torso->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	torso->_dval = 16;
	torso->_dvar = .5;

	BodyPart* tail = new BodyPart();
	tail->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	tail->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	tail->_dval = 17;
	tail->_dvar = .5;

	BodyPart* arm = new BodyPart();
	arm->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	arm->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	arm->_dval = 18;
	arm->_dvar = .5;

	BodyPart* leg = new BodyPart();
	leg->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	leg->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	leg->_dval = 19;
	leg->_dvar = .5;

	BodyPart* claw = new BodyPart();
	claw->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	claw->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	claw->_dval = 20;
	claw->_dvar = .5;

	BodyPart* wing = new BodyPart();
	wing->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	wing->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	wing->_dval = 21;
	wing->_dvar = .5;

	BodyPart* tentacle = new BodyPart();
	tentacle->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	tentacle->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	tentacle->_dval = 23;
	tentacle->_dvar = .5;

	BodyPart* uppertentacle = new BodyPart();
	uppertentacle->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	uppertentacle->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	uppertentacle->_dval = 24;
	uppertentacle->_dvar = .5;

	BodyPart* lowertentacle = new BodyPart();
	lowertentacle->_bh = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
	lowertentacle->_dtype = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	lowertentacle->_dval = 25;
	lowertentacle->_dvar = .5;




	Body* body = new Body();
	body->_body_part_table.add(0, head);
	body->_body_part_table.add(1, chest);
	body->_body_part_table.add(2, abdomen);
	body->_body_part_table.add(3, upperarm);
	body->_body_part_table.add(4, lowerarm);
	body->_body_part_table.add(5, hand);
	body->_body_part_table.add(6, upperleg);
	body->_body_part_table.add(7, lowerleg);
	body->_body_part_table.add(8, foot);
	body->_body_part_table.add(9, horn);
	body->_body_part_table.add(10, frontleg);
	body->_body_part_table.add(12, frontfoot);
	body->_body_part_table.add(13, rearleg);
	body->_body_part_table.add(15, rearfoot);
	body->_body_part_table.add(16, torso);
	body->_body_part_table.add(17, tail);
	body->_body_part_table.add(18, arm);
	body->_body_part_table.add(19, leg);
	body->_body_part_table.add(20, claw);
	body->_body_part_table.add(21, wing);
	body->_body_part_table.add(23, tentacle);
	body->_body_part_table.add(24, uppertentacle);
	body->_body_part_table.add(25, lowertentacle);

	return g_pWeenieFactory->UpdateWeenieBody(wcid, body);
}
