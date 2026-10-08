
/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
 SET NAMES utf8mb4 ;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `dragonraja_total` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci */;

USE `dragonraja_total`;
DROP TABLE IF EXISTS `cdkey`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `cdkey` (
  `CDKey` text,
  `CDKeyStatus` text,
  `CDKeyNumber` text,
  `CDKeyRegID` text,
  `CDKeyDateTime` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `cdkey` WRITE;
/*!40000 ALTER TABLE `cdkey` DISABLE KEYS */;
/*!40000 ALTER TABLE `cdkey` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `chr_info2_del`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chr_info2_del` (
  `NEW` text,
  `NAME` text,
  `LOGIN_ID` text,
  `BANKITEM` text,
  `GOD_FOOD` text,
  `GOD_FOOD_DATE` text,
  `FAITH` text,
  `BELIEVE` text,
  `EVANGELIST` text,
  `GOD_CAST_LEVEL` text,
  `BELIEVE_GOD` text,
  `BIRTHDAY` text,
  `SALVATION` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `chr_info2_del` WRITE;
/*!40000 ALTER TABLE `chr_info2_del` DISABLE KEYS */;
/*!40000 ALTER TABLE `chr_info2_del` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `chr_info_del`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chr_info_del` (
  `NAME` text,
  `LOGIN_ID` text,
  `TOTAL_ID` text,
  `BALIVE` text,
  `GENDER` text,
  `SPRNO` text,
  `RACE` text,
  `SPRITVALUE` text,
  `FACE` text,
  `CLASS` text,
  `CLASS_SPRITUALITY` text,
  `CLASS_POISONING` text,
  `CLASS_BOMBPLAY` text,
  `CLASS_ENTRAPMENT` text,
  `CLASS_SCROLLING` text,
  `CLASS_CUNNING1` text,
  `CLASS_CUNNING2` text,
  `CLASS_CUNNING3` text,
  `CLASS_STEALING` text,
  `JOB` text,
  `SPELL` text,
  `CLOTHR` text,
  `CLOTHG` text,
  `CLOTHB` text,
  `BODYR` text,
  `BODYG` text,
  `BODYB` text,
  `AGE` text,
  `X` text,
  `Y` text,
  `LEV` text,
  `EXP` text,
  `MANA` text,
  `MANAMAX` text,
  `HP` text,
  `HPMAX` text,
  `HUNGRY` text,
  `HUNGRYMAX` text,
  `GUILDNAME` text,
  `NUT1` text,
  `NUT2` text,
  `NUT3` text,
  `KILLMON` text,
  `KILLANIMAL` text,
  `KILLPC` text,
  `MONEY` text,
  `STR` text,
  `CON` text,
  `DEX` text,
  `WIS` text,
  `INT` text,
  `MOVEP` text,
  `CHAR` text,
  `ENDU` text,
  `MORAL` text,
  `LUCK` text,
  `WSPS` text,
  `NATION` text,
  `LADDERSCORE` text,
  `CONDITION` text,
  `RESIST_POISON` text,
  `RESIST_STONE` text,
  `RESIST_MAGIC` text,
  `RESIST_FIRE` text,
  `RESIST_ICE` text,
  `RESIST_ELECT` text,
  `WIZARDSPELL` text,
  `WS` text,
  `PRIESTSPELL` text,
  `PS` text,
  `BANKMONEY` text,
  `ACC_EQUIP1` text,
  `ACC_EQUIP2` text,
  `ACC_EQUIP3` text,
  `ACC_EQUIP4` text,
  `ACC_EQUIP5` text,
  `ACC_EQUIP6` text,
  `SIGHT` text,
  `INVENTORY` text,
  `TACTICS` text,
  `MAPNAME` text,
  `PEACESTS` text,
  `QUICK` text,
  `EQUIP` text,
  `BASE_HD` text,
  `SKILL` text,
  `SKILL_EXP` text,
  `SCRIPT_VAR` text,
  `OPENHOUSE` text,
  `WIN_DEFEAT` text,
  `LASTLOAN` text,
  `LASTLOAN_TIME` text,
  `RESERVED_POINT` text,
  `TAC_SKILLEXP` text,
  `VIEWTYPE` text,
  `PARTY` text,
  `RELATION` text,
  `EMPLOYMENT` text,
  `DISEASE1` text,
  `DISEASE2` text,
  `DISEASE3` text,
  `DISEASE4` text,
  `DISEASE5` text,
  `DISEASE6` text,
  `ITEMINDEX` text,
  `SOCIAL_STATUS` text,
  `FAME_PK` text,
  `FAME` text,
  `DELDATE` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `chr_info_del` WRITE;
/*!40000 ALTER TABLE `chr_info_del` DISABLE KEYS */;
/*!40000 ALTER TABLE `chr_info_del` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `chr_log_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chr_log_info` (
  `id_index` text,
  `login_id` text,
  `passwd` text,
  `passwd_hint` text,
  `d_name` text,
  `d_age` text,
  `d_sex` text,
  `d_jumin` text,
  `d_zip` text,
  `d_addr` text,
  `d_addr2` text,
  `d_tel` text,
  `d_tel2` text,
  `d_job` text,
  `d_dongi` text,
  `d_kyulje` text,
  `d_email` text,
  `d_home` text,
  `d_intro` text,
  `d_regday` text,
  `LastLogin` text,
  `LastLogout` text,
  `Remaining_Secs` text,
  `Remaining_Time` text,
  `upsize_ts` text,
  `d_sday` text,
  `d_eday` text,
  `bill_point` text,
  `mail_check` text,
  `timeremain` text,
  `recom_id` text,
  `bbs_block` text,
  `event1` text,
  `event2` text,
  `event3` text,
  `event4` text,
  `event5` text,
  `event6` text,
  `event7` text,
  `event8` text,
  `event9` text,
  `event10` text,
  `JF` text,
  `QX` text,
  KEY `idx_total_chr_login` (`login_id`(20))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `chr_log_info` WRITE;
/*!40000 ALTER TABLE `chr_log_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `chr_log_info` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `chr_log_info_??`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chr_log_info_??` (
  `id_index` text,
  `login_id` text,
  `passwd` text,
  `passwd_hint` text,
  `d_name` text,
  `d_age` text,
  `d_sex` text,
  `d_jumin` text,
  `d_zip` text,
  `d_addr` text,
  `d_tel` text,
  `d_tel2` text,
  `d_job` text,
  `d_dongi` text,
  `d_kyulje` text,
  `d_email` text,
  `d_home` text,
  `d_intro` text,
  `d_regday` text,
  `LastLogin` text,
  `LastLogout` text,
  `Remaining_Secs` text,
  `Remaining_Time` text,
  `upsize_ts` text,
  `name1` text,
  `name2` text,
  `name3` text,
  `name4` text,
  `vote` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `chr_log_info_??` WRITE;
/*!40000 ALTER TABLE `chr_log_info_??` DISABLE KEYS */;
/*!40000 ALTER TABLE `chr_log_info_??` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `chr_log_info_del`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chr_log_info_del` (
  `ID_INDEX` longtext,
  `LOGIN_ID` longtext,
  `PASSWD` longtext,
  `PASSWD_HINT` longtext,
  `D_NAME` longtext,
  `D_AGE` longtext,
  `D_SEX` longtext,
  `D_JUMIN` longtext,
  `D_ZIP` longtext,
  `D_ADDR` longtext,
  `D_ADDR2` longtext,
  `D_TEL` longtext,
  `D_TEL2` longtext,
  `D_JOB` longtext,
  `D_DONGI` longtext,
  `D_KYULJE` longtext,
  `D_EMAIL` longtext,
  `D_HOME` longtext,
  `D_INTRO` longtext,
  `D_REGDAY` longtext,
  `LASTLOGIN` longtext,
  `LASTLOGOUT` longtext,
  `REMAINING_SECS` longtext,
  `REMAINING_TIME` longtext,
  `UPSIZE_TS` longtext,
  `D_SDAY` longtext,
  `D_EDAY` longtext,
  `BILL_POINT` longtext,
  `MAIL_CHECK` longtext,
  `TIMEREMAIN` longtext,
  `RECOM_ID` longtext,
  `BBS_BLOCK` longtext,
  `EVENT1` longtext,
  `EVENT2` longtext,
  `EVENT3` longtext,
  `EVENT4` longtext,
  `EVENT5` longtext,
  `EVENT6` longtext,
  `EVENT7` longtext,
  `EVENT8` longtext,
  `EVENT9` longtext,
  `EVENT10` longtext,
  `JF` longtext,
  `QX` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `chr_log_info_del` WRITE;
/*!40000 ALTER TABLE `chr_log_info_del` DISABLE KEYS */;
/*!40000 ALTER TABLE `chr_log_info_del` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `chr_log_info_del1`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chr_log_info_del1` (
  `ID_INDEX` longtext,
  `LOGIN_ID` longtext,
  `PASSWD` longtext,
  `PASSWD_HINT` longtext,
  `D_NAME` longtext,
  `D_AGE` longtext,
  `D_SEX` longtext,
  `D_JUMIN` longtext,
  `D_ZIP` longtext,
  `D_ADDR` longtext,
  `D_TEL` longtext,
  `D_TEL2` longtext,
  `D_JOB` longtext,
  `D_DONGI` longtext,
  `D_KYULJE` longtext,
  `D_EMAIL` longtext,
  `D_HOME` longtext,
  `D_INTRO` longtext,
  `D_REGDAY` longtext,
  `LASTLOGIN` longtext,
  `LASTLOGOUT` longtext,
  `REMAINING_SECS` longtext,
  `REMAINING_TIME` longtext,
  `UPSIZE_TS` longtext,
  `NAME1` longtext,
  `NAME2` longtext,
  `NAME3` longtext,
  `NAME4` longtext,
  `VOTE` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `chr_log_info_del1` WRITE;
/*!40000 ALTER TABLE `chr_log_info_del1` DISABLE KEYS */;
/*!40000 ALTER TABLE `chr_log_info_del1` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `chr_log_info_원본`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chr_log_info_원본` (
  `id_index` text,
  `login_id` text,
  `passwd` text,
  `passwd_hint` text,
  `d_name` text,
  `d_age` text,
  `d_sex` text,
  `d_jumin` text,
  `d_zip` text,
  `d_addr` text,
  `d_tel` text,
  `d_tel2` text,
  `d_job` text,
  `d_dongi` text,
  `d_kyulje` text,
  `d_email` text,
  `d_home` text,
  `d_intro` text,
  `d_regday` text,
  `LastLogin` text,
  `LastLogout` text,
  `Remaining_Secs` text,
  `Remaining_Time` text,
  `upsize_ts` text,
  `name1` text,
  `name2` text,
  `name3` text,
  `name4` text,
  `vote` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `chr_log_info_원본` WRITE;
/*!40000 ALTER TABLE `chr_log_info_원본` DISABLE KEYS */;
/*!40000 ALTER TABLE `chr_log_info_원본` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `dtproperties`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `dtproperties` (
  `id` longtext,
  `objectid` longtext,
  `property` longtext,
  `value` longtext,
  `uvalue` longtext,
  `lvalue` longtext,
  `version` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `dtproperties` WRITE;
/*!40000 ALTER TABLE `dtproperties` DISABLE KEYS */;
/*!40000 ALTER TABLE `dtproperties` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `ip_account`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `ip_account` (
  `ip_idx` varchar(255) DEFAULT NULL,
  `ip_login_id` varchar(255) DEFAULT NULL,
  `ip_login_pw` varchar(255) DEFAULT NULL,
  `ip_billing_type` varchar(255) DEFAULT NULL,
  `ip_billing_sday` varchar(255) DEFAULT NULL,
  `ip_billing_eday` varchar(255) DEFAULT NULL,
  `ip_name` varchar(255) DEFAULT NULL,
  `ip_jumin` varchar(255) DEFAULT NULL,
  `ip_bname` varchar(255) DEFAULT NULL,
  `ip_bnum` varchar(255) DEFAULT NULL,
  `ip_email` varchar(255) DEFAULT NULL,
  `ip_tel1` varchar(255) DEFAULT NULL,
  `ip_tel2` varchar(255) DEFAULT NULL,
  `ip_count` varchar(255) DEFAULT NULL,
  `ip_zipcode` varchar(255) DEFAULT NULL,
  `ip_addr` varchar(255) DEFAULT NULL,
  `ip_regday` varchar(255) DEFAULT NULL,
  `ip_point` varchar(255) DEFAULT NULL,
  `speed` varchar(255) DEFAULT NULL,
  `isp` varchar(255) DEFAULT NULL,
  `content` varchar(255) DEFAULT NULL,
  `is_dr` varchar(255) DEFAULT NULL,
  `visited` varchar(255) DEFAULT NULL,
  `ip_maketing` varchar(255) DEFAULT NULL,
  `totaltime` varchar(255) DEFAULT NULL,
  `time1` varchar(255) DEFAULT NULL,
  `timeremain` varchar(255) DEFAULT NULL,
  `timeday` varchar(255) DEFAULT NULL,
  `timenight` varchar(255) DEFAULT NULL,
  `login_ip` varchar(255) DEFAULT NULL,
  `ip_type` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `ip_account` WRITE;
/*!40000 ALTER TABLE `ip_account` DISABLE KEYS */;
/*!40000 ALTER TABLE `ip_account` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `ip_use`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `ip_use` (
  `no` varchar(255) DEFAULT NULL,
  `ip_idx` varchar(255) DEFAULT NULL,
  `ip` varchar(255) DEFAULT NULL,
  `billing_sday` varchar(255) DEFAULT NULL,
  `billing_eday` varchar(255) DEFAULT NULL,
  `use_check` varchar(255) DEFAULT NULL,
  `can_use` varchar(255) DEFAULT NULL,
  `request_use` varchar(255) DEFAULT NULL,
  `card_use` varchar(255) DEFAULT NULL,
  `ip_regday` varchar(255) DEFAULT NULL,
  `ip_type` varchar(255) DEFAULT NULL,
  `time_check` varchar(255) DEFAULT NULL,
  KEY `idx_ip_use_ip` (`ip`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `ip_use` WRITE;
/*!40000 ALTER TABLE `ip_use` DISABLE KEYS */;
/*!40000 ALTER TABLE `ip_use` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `login_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `login_log` (
  `serial` longtext,
  `login_id` longtext,
  `type` longtext,
  `status` longtext,
  `time` longtext,
  `ip` longtext,
  `joint_id` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `login_log` WRITE;
/*!40000 ALTER TABLE `login_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `login_log` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `logintab`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `logintab` (
  `???` varchar(255) DEFAULT NULL,
  `ID` varchar(255) DEFAULT NULL,
  `logintime` varchar(255) DEFAULT NULL,
  `logouttime` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `logintab` WRITE;
/*!40000 ALTER TABLE `logintab` DISABLE KEYS */;
/*!40000 ALTER TABLE `logintab` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `logintable`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `logintable` (
  `servername` varchar(255) DEFAULT NULL,
  `user_id` varchar(255) DEFAULT NULL,
  `type` varchar(255) DEFAULT NULL,
  `ip` varchar(255) DEFAULT NULL,
  `joint_id` varchar(255) DEFAULT NULL,
  `port` varchar(255) DEFAULT NULL,
  `agent_id` varchar(255) DEFAULT NULL,
  `server_set_num` varchar(255) DEFAULT NULL,
  `d_kyulje` varchar(255) DEFAULT NULL,
  KEY `idx_logintable_uid` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `logintable` WRITE;
/*!40000 ALTER TABLE `logintable` DISABLE KEYS */;
/*!40000 ALTER TABLE `logintable` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `pointtab`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `pointtab` (
  `???` varchar(255) DEFAULT NULL,
  `ID` varchar(255) DEFAULT NULL,
  `pointtype` varchar(255) DEFAULT NULL,
  `point` varchar(255) DEFAULT NULL,
  `expiredate` varchar(255) DEFAULT NULL,
  `lastupdate` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `pointtab` WRITE;
/*!40000 ALTER TABLE `pointtab` DISABLE KEYS */;
/*!40000 ALTER TABLE `pointtab` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `rm_authority_set`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `rm_authority_set` (
  `type_name` varchar(255) DEFAULT NULL,
  `type_value` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `rm_authority_set` WRITE;
/*!40000 ALTER TABLE `rm_authority_set` DISABLE KEYS */;
/*!40000 ALTER TABLE `rm_authority_set` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `rm_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `rm_log` (
  `Index` longtext,
  `ID` longtext,
  `Type` longtext,
  `Date` longtext,
  `ClientIp` longtext,
  `Log` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `rm_log` WRITE;
/*!40000 ALTER TABLE `rm_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `rm_log` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `rm_log_extension`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `rm_log_extension` (
  `index` longtext,
  `ID` longtext,
  `data` longtext,
  `ServerName` longtext,
  `DbName` longtext,
  `func` longtext,
  `page` longtext,
  `ch_name` longtext,
  `ch_id` longtext,
  `Typename` longtext,
  `value` longtext,
  `tmp` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `rm_log_extension` WRITE;
/*!40000 ALTER TABLE `rm_log_extension` DISABLE KEYS */;
/*!40000 ALTER TABLE `rm_log_extension` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `rm_log_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `rm_log_info` (
  `ID` varchar(255) DEFAULT NULL,
  `Password` varchar(255) DEFAULT NULL,
  `Name` varchar(255) DEFAULT NULL,
  `LoginType` varchar(255) DEFAULT NULL,
  `IP` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `rm_log_info` WRITE;
/*!40000 ALTER TABLE `rm_log_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `rm_log_info` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `rm_now_repair`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `rm_now_repair` (
  `User_Id` varchar(255) DEFAULT NULL,
  `serversetNum` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `rm_now_repair` WRITE;
/*!40000 ALTER TABLE `rm_now_repair` DISABLE KEYS */;
/*!40000 ALTER TABLE `rm_now_repair` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `rm_server_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `rm_server_info` (
  `MachineName` varchar(255) DEFAULT NULL,
  `Ip` varchar(255) DEFAULT NULL,
  `Port` varchar(255) DEFAULT NULL,
  `MapName` varchar(255) DEFAULT NULL,
  `ServerType` varchar(255) DEFAULT NULL,
  `ServerSetNum` varchar(255) DEFAULT NULL,
  `RMToolListenPort` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `rm_server_info` WRITE;
/*!40000 ALTER TABLE `rm_server_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `rm_server_info` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `rm_server_info2`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `rm_server_info2` (
  `MachineName` varchar(255) DEFAULT NULL,
  `Ip` varchar(255) DEFAULT NULL,
  `Port` varchar(255) DEFAULT NULL,
  `MapName` varchar(255) DEFAULT NULL,
  `ServerType` varchar(255) DEFAULT NULL,
  `ServerSetNum` varchar(255) DEFAULT NULL,
  `RMToolListenPort` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `rm_server_info2` WRITE;
/*!40000 ALTER TABLE `rm_server_info2` DISABLE KEYS */;
/*!40000 ALTER TABLE `rm_server_info2` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `rm_server_info_kyo`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `rm_server_info_kyo` (
  `MachineName` varchar(255) DEFAULT NULL,
  `Ip` varchar(255) DEFAULT NULL,
  `Port` varchar(255) DEFAULT NULL,
  `MapName` varchar(255) DEFAULT NULL,
  `ServerType` varchar(255) DEFAULT NULL,
  `ServerSetNum` varchar(255) DEFAULT NULL,
  `RMToolListenPort` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `rm_server_info_kyo` WRITE;
/*!40000 ALTER TABLE `rm_server_info_kyo` DISABLE KEYS */;
/*!40000 ALTER TABLE `rm_server_info_kyo` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `rm_server_status`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `rm_server_status` (
  `MachineName` varchar(255) DEFAULT NULL,
  `IP` varchar(255) DEFAULT NULL,
  `Port` varchar(255) DEFAULT NULL,
  `MapName` varchar(255) DEFAULT NULL,
  `ServerType` varchar(255) DEFAULT NULL,
  `ServerStatus` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `rm_server_status` WRITE;
/*!40000 ALTER TABLE `rm_server_status` DISABLE KEYS */;
/*!40000 ALTER TABLE `rm_server_status` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `time_channeli`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `time_channeli` (
  `joint_id` longtext,
  `login_id` longtext,
  `totaltime` longtext,
  `time1` longtext,
  `stime` longtext,
  `etime` longtext,
  `stimes` longtext,
  `etimes` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `time_channeli` WRITE;
/*!40000 ALTER TABLE `time_channeli` DISABLE KEYS */;
/*!40000 ALTER TABLE `time_channeli` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `time_ip`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `time_ip` (
  `ip` varchar(255) DEFAULT NULL,
  `ip_idx` varchar(255) DEFAULT NULL,
  `totaltime` varchar(255) DEFAULT NULL,
  `time1` varchar(255) DEFAULT NULL,
  `stime` varchar(255) DEFAULT NULL,
  `etime` varchar(255) DEFAULT NULL,
  `stimes` varchar(255) DEFAULT NULL,
  `etimes` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `time_ip` WRITE;
/*!40000 ALTER TABLE `time_ip` DISABLE KEYS */;
/*!40000 ALTER TABLE `time_ip` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `time_joyque`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `time_joyque` (
  `joyque_idx` longtext,
  `login_id` longtext,
  `stime` longtext,
  `etime` longtext,
  `totaltime` longtext,
  `time1` longtext,
  `stimes` longtext,
  `etimes` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `time_joyque` WRITE;
/*!40000 ALTER TABLE `time_joyque` DISABLE KEYS */;
/*!40000 ALTER TABLE `time_joyque` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `time_kornetworld`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `time_kornetworld` (
  `joint_id` longtext,
  `login_id` longtext,
  `totaltime` longtext,
  `time1` longtext,
  `stime` longtext,
  `etime` longtext,
  `stimes` longtext,
  `etimes` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `time_kornetworld` WRITE;
/*!40000 ALTER TABLE `time_kornetworld` DISABLE KEYS */;
/*!40000 ALTER TABLE `time_kornetworld` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `time_mezzy`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `time_mezzy` (
  `joint_id` longtext,
  `login_id` longtext,
  `totaltime` longtext,
  `time1` longtext,
  `stime` longtext,
  `etime` longtext,
  `stimes` longtext,
  `etimes` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `time_mezzy` WRITE;
/*!40000 ALTER TABLE `time_mezzy` DISABLE KEYS */;
/*!40000 ALTER TABLE `time_mezzy` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `time_netsgo`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `time_netsgo` (
  `joint_id` varchar(255) DEFAULT NULL,
  `login_id` varchar(255) DEFAULT NULL,
  `totaltime` varchar(255) DEFAULT NULL,
  `time1` varchar(255) DEFAULT NULL,
  `stime` varchar(255) DEFAULT NULL,
  `etime` varchar(255) DEFAULT NULL,
  `stimes` varchar(255) DEFAULT NULL,
  `etimes` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `time_netsgo` WRITE;
/*!40000 ALTER TABLE `time_netsgo` DISABLE KEYS */;
/*!40000 ALTER TABLE `time_netsgo` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `time_nio`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `time_nio` (
  `ip` varchar(255) DEFAULT NULL,
  `totaltime` varchar(255) DEFAULT NULL,
  `time1` varchar(255) DEFAULT NULL,
  `stime` varchar(255) DEFAULT NULL,
  `etime` varchar(255) DEFAULT NULL,
  `stimes` varchar(255) DEFAULT NULL,
  `etimes` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `time_nio` WRITE;
/*!40000 ALTER TABLE `time_nio` DISABLE KEYS */;
/*!40000 ALTER TABLE `time_nio` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `time_thrunet`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `time_thrunet` (
  `joint_id` longtext,
  `login_id` longtext,
  `totaltime` longtext,
  `time1` longtext,
  `stime` longtext,
  `etime` longtext,
  `stimes` longtext,
  `etimes` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `time_thrunet` WRITE;
/*!40000 ALTER TABLE `time_thrunet` DISABLE KEYS */;
/*!40000 ALTER TABLE `time_thrunet` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `time_type`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `time_type` (
  `type` varchar(255) DEFAULT NULL,
  `des` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `time_type` WRITE;
/*!40000 ALTER TABLE `time_type` DISABLE KEYS */;
/*!40000 ALTER TABLE `time_type` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `time_unitel`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `time_unitel` (
  `joint_id` varchar(255) DEFAULT NULL,
  `login_id` varchar(255) DEFAULT NULL,
  `totaltime` varchar(255) DEFAULT NULL,
  `time1` varchar(255) DEFAULT NULL,
  `stime` varchar(255) DEFAULT NULL,
  `etime` varchar(255) DEFAULT NULL,
  `stimes` varchar(255) DEFAULT NULL,
  `etimes` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `time_unitel` WRITE;
/*!40000 ALTER TABLE `time_unitel` DISABLE KEYS */;
/*!40000 ALTER TABLE `time_unitel` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `web_admin`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `web_admin` (
  `ACC` varchar(255) DEFAULT NULL,
  `PASS` varchar(255) DEFAULT NULL,
  `QX` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `web_admin` WRITE;
/*!40000 ALTER TABLE `web_admin` DISABLE KEYS */;
/*!40000 ALTER TABLE `web_admin` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `web_adminhistory`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `web_adminhistory` (
  `ID` longtext,
  `ADMIN` longtext,
  `ACC` longtext,
  `TYPE` longtext,
  `TITLE` longtext,
  `BODY` longtext,
  `DATE` longtext,
  `READS` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `web_adminhistory` WRITE;
/*!40000 ALTER TABLE `web_adminhistory` DISABLE KEYS */;
/*!40000 ALTER TABLE `web_adminhistory` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `web_buymoneyclass`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `web_buymoneyclass` (
  `ID` longtext,
  `TYPE` longtext,
  `TITLE` longtext,
  `DATE` longtext,
  `POINT` longtext,
  `MONEY` longtext,
  `QX` longtext,
  `JF` longtext,
  `ZBCODE` longtext,
  `ZBNAME` longtext,
  `ZBPRICE` longtext,
  `BZ` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `web_buymoneyclass` WRITE;
/*!40000 ALTER TABLE `web_buymoneyclass` DISABLE KEYS */;
/*!40000 ALTER TABLE `web_buymoneyclass` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `web_card`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `web_card` (
  `ID` longtext,
  `CARDNUMBER` longtext,
  `CARDPASSWORD` longtext,
  `CARDTYPE` longtext,
  `CARDUNIT` longtext,
  `USEUSERNAME` longtext,
  `USEIP` longtext,
  `USEDATETIME` longtext,
  `ISUSED` longtext,
  `DATETIME` longtext,
  `CARDTYPEID` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `web_card` WRITE;
/*!40000 ALTER TABLE `web_card` DISABLE KEYS */;
/*!40000 ALTER TABLE `web_card` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `web_ddb`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `web_ddb` (
  `ID` longtext,
  `YHID` longtext,
  `ACC` longtext,
  `MONEY` longtext,
  `FKFS` longtext,
  `ZT` longtext,
  `CPLX` longtext,
  `CPID` longtext,
  `D_DATE` longtext,
  `F_DATE` longtext,
  `ZFFS` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `web_ddb` WRITE;
/*!40000 ALTER TABLE `web_ddb` DISABLE KEYS */;
/*!40000 ALTER TABLE `web_ddb` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `web_download`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `web_download` (
  `ID` longtext,
  `NAME` longtext,
  `EXPLAIN` longtext,
  `DOWNLOADURL` longtext,
  `ADDTIME` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `web_download` WRITE;
/*!40000 ALTER TABLE `web_download` DISABLE KEYS */;
/*!40000 ALTER TABLE `web_download` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `web_dp`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `web_dp` (
  `ID` longtext,
  `BODY` longtext,
  `DPCS` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `web_dp` WRITE;
/*!40000 ALTER TABLE `web_dp` DISABLE KEYS */;
/*!40000 ALTER TABLE `web_dp` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `web_message`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `web_message` (
  `ID` longtext,
  `TITLE` longtext,
  `BODY` longtext,
  `DATE` longtext,
  `FROMUSER` longtext,
  `TOUSER` longtext,
  `READS` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `web_message` WRITE;
/*!40000 ALTER TABLE `web_message` DISABLE KEYS */;
/*!40000 ALTER TABLE `web_message` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `web_news`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `web_news` (
  `ID` longtext,
  `TITLE` longtext,
  `BODY` longtext,
  `DATE` longtext,
  `LX` longtext,
  `ACC` longtext,
  `YZ` longtext,
  `CLICKCS` longtext,
  `TJ` longtext
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `web_news` WRITE;
/*!40000 ALTER TABLE `web_news` DISABLE KEYS */;
/*!40000 ALTER TABLE `web_news` ENABLE KEYS */;
UNLOCK TABLES;
/*!50003 DROP PROCEDURE IF EXISTS `up_check_user_id` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=CURRENT_USER PROCEDURE `up_check_user_id`(IN p_user_id VARCHAR(20) CHARACTER SET utf8mb4)
BEGIN
    SELECT user_id, port, agent_id, server_set_num
    FROM logintable WHERE user_id = p_user_id;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `up_get_ip_info` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=CURRENT_USER PROCEDURE `up_get_ip_info`(IN p_ip VARCHAR(20) CHARACTER SET utf8mb4)
BEGIN
    SELECT can_use, billing_eday, ip_type, ip_idx
    FROM ip_use WHERE ip = p_ip;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `up_get_user_info2` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=CURRENT_USER PROCEDURE `up_get_user_info2`(
    IN p_login_id VARCHAR(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci
)
BEGIN
    SELECT CAST(id_index AS UNSIGNED) AS id_index,
           passwd,
           CAST(d_kyulje AS UNSIGNED) AS d_kyulje,
           CAST(d_eday AS DATETIME) AS d_eday,
           CAST(timeremain AS UNSIGNED) AS timeremain
    FROM chr_log_info
    WHERE login_id = p_login_id;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

