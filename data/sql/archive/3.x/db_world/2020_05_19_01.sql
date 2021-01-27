-- DB update 2020_05_19_00 -> 2020_05_19_01
DROP PROCEDURE IF EXISTS `updateDb`;
DELIMITER //
CREATE PROCEDURE updateDb ()
proc:BEGIN DECLARE OK VARCHAR(100) DEFAULT 'FALSE';
SELECT COUNT(*) INTO @COLEXISTS
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'version_db_world' AND COLUMN_NAME = '2020_05_19_00';
IF @COLEXISTS = 0 THEN LEAVE proc; END IF;
START TRANSACTION;
ALTER TABLE version_db_world CHANGE COLUMN 2020_05_19_00 2020_05_19_01 bit;
SELECT sql_rev INTO OK FROM version_db_world WHERE sql_rev = '1587299483794804200'; IF OK <> 'FALSE' THEN LEAVE proc; END IF;
--
-- START UPDATING QUERIES
--

INSERT INTO `version_db_world` (`sql_rev`) VALUES ('1587299483794804200');

UPDATE `gameobject` SET `position_x` = 792.891, `position_y` = -5149.93, `position_z` = -82.3544, `orientation` = 2.11791, `spawntimesecs` = 5 WHERE `guid` = 56732 AND `guid` = 56732;

--
-- END UPDATING QUERIES
--
COMMIT;
END //
DELIMITER ;
CALL updateDb();
DROP PROCEDURE IF EXISTS `updateDb`;