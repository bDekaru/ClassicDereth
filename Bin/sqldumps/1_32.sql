CREATE TABLE `character_corpses` (
	`character_id` INT(10) UNSIGNED NOT NULL,
	`corpses` VARCHAR(4096) NULL DEFAULT NULL,
	PRIMARY KEY (`character_id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;
