ALTER TABLE `characters`
	ADD COLUMN `gag_timer` INT(11) NULL DEFAULT '0' AFTER `ts_login`,
	ADD COLUMN `gag_count` INT(11) NULL DEFAULT '0' AFTER `gag_timer`;
	