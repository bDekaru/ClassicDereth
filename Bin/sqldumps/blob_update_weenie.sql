DELIMITER //
CREATE DEFINER=`root`@`localhost` PROCEDURE `blob_update_weenie`(
    IN weenieId INT(11) UNSIGNED,
    IN topLevelId INT(11) UNSIGNED,
    IN blockId INT(11) UNSIGNED,
    IN weenieData MEDIUMBLOB
)
BEGIN

INSERT INTO weenies (id, top_level_object_id, block_id, DATA)
    VALUES (weenieId, topLevelId, blockId, weenieData)
    ON DUPLICATE KEY UPDATE
        top_level_object_id=topLevelId,
        block_id=blockId,
        DATA=weenieData;
END //
DELIMITER ;