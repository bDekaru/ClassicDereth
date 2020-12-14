CREATE TABLE `character_windowdata` (
	`character_id` INT(11) NOT NULL DEFAULT '0',
	`bloblength` INT(11) NULL DEFAULT NULL,
	`windowblob` BLOB NULL DEFAULT NULL,
	PRIMARY KEY (`character_id`)
)
ENGINE=InnoDB
;
