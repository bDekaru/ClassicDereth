ALTER TABLE `accounts`
	ADD COLUMN `email` VARCHAR(64) NULL DEFAULT '' AFTER `created_ip_address`;

ALTER TABLE `accounts`
	ADD COLUMN `emailsetused` BIT(1) NOT NULL DEFAULT b'0' AFTER `email`;

ALTER TABLE `characters`
	ADD COLUMN `ts_deleted` INT(11) NOT NULL DEFAULT '0' AFTER `instance_ts`,
	ADD COLUMN `ts_login` INT(11) NOT NULL DEFAULT '0' AFTER `ts_deleted`;
