CREATE TABLE `character_friends` (
	`character_id` INT(11) UNSIGNED NOT NULL,
	`friend_type` INT(11) UNSIGNED NOT NULL,
	`friend_id` INT(11) UNSIGNED NOT NULL,
	PRIMARY KEY (`character_id`, `friend_type`, `friend_id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;
