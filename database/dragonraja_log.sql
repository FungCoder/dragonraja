
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

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `dragonraja_log` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci */;

USE `dragonraja_log`;
DROP TABLE IF EXISTS `accessablegmtoolipaddress`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `accessablegmtoolipaddress` (
  `ip` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `accessablegmtoolipaddress` WRITE;
/*!40000 ALTER TABLE `accessablegmtoolipaddress` DISABLE KEYS */;
/*!40000 ALTER TABLE `accessablegmtoolipaddress` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `chr_log_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `chr_log_info` (
  `id_index` varchar(255) DEFAULT NULL,
  `login_id` varchar(255) DEFAULT NULL,
  `passwd` varchar(255) DEFAULT NULL,
  `passwd_hint` varchar(255) DEFAULT NULL,
  `d_name` varchar(255) DEFAULT NULL,
  `d_age` varchar(255) DEFAULT NULL,
  `d_sex` varchar(255) DEFAULT NULL,
  `d_jumin` varchar(255) DEFAULT NULL,
  `d_zip` varchar(255) DEFAULT NULL,
  `d_addr` varchar(255) DEFAULT NULL,
  `d_tel` varchar(255) DEFAULT NULL,
  `d_tel2` varchar(255) DEFAULT NULL,
  `d_job` varchar(255) DEFAULT NULL,
  `d_dongi` varchar(255) DEFAULT NULL,
  `d_kyulje` varchar(255) DEFAULT NULL,
  `d_email` varchar(255) DEFAULT NULL,
  `d_home` varchar(255) DEFAULT NULL,
  `d_intro` varchar(255) DEFAULT NULL,
  `d_regday` varchar(255) DEFAULT NULL,
  `LastLogin` varchar(255) DEFAULT NULL,
  `LastLogout` varchar(255) DEFAULT NULL,
  `Remaining_Secs` varchar(255) DEFAULT NULL,
  `Remaining_Time` varchar(255) DEFAULT NULL,
  `upsize_ts` varchar(255) DEFAULT NULL,
  `d_sday` varchar(255) DEFAULT NULL,
  `d_eday` varchar(255) DEFAULT NULL,
  `bill_point` varchar(255) DEFAULT NULL,
  `mail_check` varchar(255) DEFAULT NULL,
  `timeremain` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `chr_log_info` WRITE;
/*!40000 ALTER TABLE `chr_log_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `chr_log_info` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `doubtuser`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `doubtuser` (
  `id` text,
  `name` text,
  `date` text,
  `type` text,
  `ip` text,
  `cause` text,
  KEY `idx_doubtuser_date` (`date`(10))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `doubtuser` WRITE;
/*!40000 ALTER TABLE `doubtuser` DISABLE KEYS */;
/*!40000 ALTER TABLE `doubtuser` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `dtproperties`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `dtproperties` (
  `id` text,
  `objectid` text,
  `property` text,
  `value` text,
  `uvalue` text,
  `lvalue` text,
  `version` text
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
  `ip` varchar(255) DEFAULT NULL,
  `ip_idx` varchar(255) DEFAULT NULL,
  `Billing_sday` varchar(255) DEFAULT NULL,
  `Billing_eday` varchar(255) DEFAULT NULL,
  `Use_check` varchar(255) DEFAULT NULL,
  `Can_use` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `ip_use` WRITE;
/*!40000 ALTER TABLE `ip_use` DISABLE KEYS */;
/*!40000 ALTER TABLE `ip_use` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `itemlog`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `itemlog` (
  `date` text,
  `map` text,
  `maker` text,
  `itemno` text,
  `today_count` text,
  `grade` text,
  `mutanttype1` text,
  `mutanttype2` text,
  `addeditem1` text,
  `addeditem1limit` text,
  `addeditem2` text,
  `addeditem2limit` text,
  `resultlimit` text,
  `resultnowdur` text,
  `resultmaxdur` text,
  `why` text,
  `why2` text,
  `resource1limit` text,
  `resource2limit` text,
  `resource3limit` text,
  `resource4limit` text,
  `resource5limit` text,
  `resource6limit` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `itemlog` WRITE;
/*!40000 ALTER TABLE `itemlog` DISABLE KEYS */;
/*!40000 ALTER TABLE `itemlog` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `log_item_event`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `log_item_event` (
  `item_no` int(11) NOT NULL,
  `attr1` int(11) DEFAULT NULL,
  `attr2` int(11) DEFAULT NULL,
  `attr3` int(11) DEFAULT NULL,
  `attr4` int(11) DEFAULT NULL,
  `attr5` int(11) DEFAULT NULL,
  `attr6` int(11) DEFAULT NULL,
  `port` int(11) DEFAULT NULL,
  `type` int(11) DEFAULT NULL,
  `name1` varchar(50) DEFAULT NULL,
  `name2` varchar(50) DEFAULT NULL,
  `lv` int(11) DEFAULT NULL,
  `server_set_num` int(11) DEFAULT NULL,
  `date` datetime NOT NULL,
  KEY `idx_date` (`date`),
  KEY `idx_lie_date` (`date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `log_item_event` WRITE;
/*!40000 ALTER TABLE `log_item_event` DISABLE KEYS */;
/*!40000 ALTER TABLE `log_item_event` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `login_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `login_log` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `login_id` varchar(30) DEFAULT '',
  `char_name` varchar(30) DEFAULT '',
  `ip` varchar(15) DEFAULT '',
  `action` varchar(50) DEFAULT '',
  `log_time` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_login_id` (`login_id`),
  KEY `idx_log_time` (`log_time`)
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
  `servername` text,
  `user_id` text,
  `type` text,
  `ip` text,
  `joint_id` text,
  `port` text,
  `agent_id` text,
  `server_set_num` text
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
DROP TABLE IF EXISTS `rareitemlog`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `rareitemlog` (
  `date` varchar(255) DEFAULT NULL,
  `id` varchar(255) DEFAULT NULL,
  `name` varchar(255) DEFAULT NULL,
  `mapname` varchar(255) DEFAULT NULL,
  `itemno` varchar(255) DEFAULT NULL,
  `kind` varchar(255) DEFAULT NULL,
  `class` varchar(255) DEFAULT NULL,
  `type` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `rareitemlog` WRITE;
/*!40000 ALTER TABLE `rareitemlog` DISABLE KEYS */;
/*!40000 ALTER TABLE `rareitemlog` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `rarelog`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `rarelog` (
  `date` text,
  `id` text,
  `name` text,
  `mapname` text,
  `itemno` text,
  `kind` text,
  `class` text,
  `type` text,
  `why` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `rarelog` WRITE;
/*!40000 ALTER TABLE `rarelog` DISABLE KEYS */;
/*!40000 ALTER TABLE `rarelog` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `sadonixcontrol`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `sadonixcontrol` (
  `No` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `sadonixcontrol` WRITE;
/*!40000 ALTER TABLE `sadonixcontrol` DISABLE KEYS */;
/*!40000 ALTER TABLE `sadonixcontrol` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `sadonixcontrolorigin`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `sadonixcontrolorigin` (
  `No` varchar(255) DEFAULT NULL,
  `0` varchar(255) DEFAULT NULL,
  `1` varchar(255) DEFAULT NULL,
  `2` varchar(255) DEFAULT NULL,
  `3` varchar(255) DEFAULT NULL,
  `4` varchar(255) DEFAULT NULL,
  `5` varchar(255) DEFAULT NULL,
  `6` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `sadonixcontrolorigin` WRITE;
/*!40000 ALTER TABLE `sadonixcontrolorigin` DISABLE KEYS */;
/*!40000 ALTER TABLE `sadonixcontrolorigin` ENABLE KEYS */;
UNLOCK TABLES;
DROP TABLE IF EXISTS `time_channeli`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8mb4 ;
CREATE TABLE `time_channeli` (
  `joint_id` text,
  `login_id` text,
  `totaltime` text,
  `time1` text,
  `stime` text,
  `etime` text,
  `stimes` text,
  `etimes` text
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
  `joyque_idx` text,
  `login_id` text,
  `stime` text,
  `etime` text,
  `totaltime` text,
  `time1` text,
  `stimes` text,
  `etimes` text
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
  `joint_id` text,
  `login_id` text,
  `totaltime` text,
  `time1` text,
  `stime` text,
  `etime` text,
  `stimes` text,
  `etimes` text
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
  `joint_id` text,
  `login_id` text,
  `totaltime` text,
  `time1` text,
  `stime` text,
  `etime` text,
  `stimes` text,
  `etimes` text
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
  `joint_id` text,
  `login_id` text,
  `totaltime` text,
  `time1` text,
  `stime` text,
  `etime` text,
  `stimes` text,
  `etimes` text
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
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

