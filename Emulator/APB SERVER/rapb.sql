-- Database for the rAPB emulator (see Configs/Database.xml).
-- The servers create and update these tables themselves from the MyDB classes;
-- this script only exists so a fresh setup has the database and a first account.
--
-- Import:  mysql -u root -p < rapb.sql

CREATE DATABASE IF NOT EXISTS `rapb` DEFAULT CHARACTER SET latin1;
USE `rapb`;

CREATE TABLE IF NOT EXISTS `accounts` (
  `Index` int(11) NOT NULL AUTO_INCREMENT,
  `Username` text,
  `Password` text,
  `Verifier` text,
  `Salt` text,
  `Threat` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `IsAdmin` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `IsBanned` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `InUse` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `CanHostDistrict` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `Token` text,
  PRIMARY KEY (`Index`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `characters` (
  `Index` int(11) NOT NULL AUTO_INCREMENT,
  `AccountIndex` int(11) NOT NULL DEFAULT 0,
  `Name` text,
  `Slot` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `Gender` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `Faction` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `Rank` int(11) NOT NULL DEFAULT 0,
  `Money` int(11) NOT NULL DEFAULT 0,
  `JokerTickets` int(11) NOT NULL DEFAULT 0,
  `Playtime` int(11) NOT NULL DEFAULT 0,
  `Clan` text,
  `Version` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `Appearance` text,
  `World` int(11) NOT NULL DEFAULT 0,
  `IsOnline` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `DistrictID` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `DistrictType` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `LFG` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `GroupStatus` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `IsGroupPublic` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  `GroupInvite` tinyint(4) UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`Index`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `friends` (
  `Index` int(11) NOT NULL AUTO_INCREMENT,
  `CharacterOwnerIndex` int(11) NOT NULL DEFAULT 0,
  `CharacterFriendIndex` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`Index`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `ignores` (
  `Index` int(11) NOT NULL AUTO_INCREMENT,
  `CharacterOwnerIndex` int(11) NOT NULL DEFAULT 0,
  `CharacterIgnoreIndex` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`Index`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Local test account. Verifier and Salt stay empty: the lobby fills them in on the first login.
-- CanHostDistrict + Token let DistrictServer register with the token from Configs/token.id.
INSERT INTO `accounts` (`Index`, `Username`, `Password`, `Verifier`, `Salt`, `IsAdmin`, `CanHostDistrict`, `Token`)
SELECT 1, 'test', 'test', '', '', 1, 1, '90940867' FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM `accounts` WHERE `Index` = 1);
