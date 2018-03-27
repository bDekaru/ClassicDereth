-- phpMyAdmin SQL Dump
-- version 4.6.4
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Mar 27, 2018 at 01:47 PM
-- Server version: 5.7.14
-- PHP Version: 5.6.25

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `phatac`
--

-- --------------------------------------------------------

--
-- Table structure for table `accounts`
--

CREATE TABLE `accounts` (
  `id` int(11) NOT NULL,
  `username` varchar(40) NOT NULL,
  `password` varchar(129) NOT NULL,
  `password_salt` varchar(17) NOT NULL,
  `date_created` int(11) NOT NULL COMMENT 'unix timestamp',
  `access` int(11) NOT NULL COMMENT '0=basic 3=advocate 4=sentinel 6=admin',
  `created_ip_address` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

INSERT INTO `accounts` (`id`, `username`, `password`, `password_salt`, `date_created`, `access`, `created_ip_address`) VALUES
(1, 'admin', '', '', UNIX_TIMESTAMP(), 6, '');

-- --------------------------------------------------------

--
-- Table structure for table `blocks`
--

CREATE TABLE `blocks` (
  `block_id` int(11) UNSIGNED NOT NULL,
  `weenie_id` int(11) UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `characters`
--

CREATE TABLE `characters` (
  `account_id` int(11) NOT NULL,
  `weenie_id` int(11) UNSIGNED NOT NULL,
  `name` varchar(64) NOT NULL,
  `date_created` int(11) NOT NULL,
  `instance_ts` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `globals`
--

CREATE TABLE `globals` (
  `id` int(11) NOT NULL,
  `data` longblob NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `houses`
--

CREATE TABLE `houses` (
  `house_id` int(11) NOT NULL,
  `data` longblob NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `teletowns`
--

CREATE TABLE `teletowns` (
  `ID` int(11) NOT NULL,
  `Description` varchar(50) NOT NULL,
  `Command` varchar(50) NOT NULL,
  `Landblock` varchar(50) NOT NULL,
  `Position_X` varchar(50) NOT NULL,
  `Position_Y` varchar(50) NOT NULL,
  `Position_Z` varchar(50) NOT NULL,
  `Orientation_W` varchar(50) NOT NULL,
  `Orientation_X` varchar(50) NOT NULL,
  `Orientation_Y` varchar(50) NOT NULL,
  `Orientation_Z` varchar(50) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `teletowns`
--

INSERT INTO `teletowns` (`ID`, `Description`, `Command`, `Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z`) VALUES
(1, 'Teletown - Aerlinthe', 'Aerlinthe Island', 'BAE8001D', '42A80000', '42D20000', '41D00A3D', '00000000', '00000000', '00000000', 'BF800000'),
(2, 'Teletown - Ahurenga', 'Ahurenga', '0FB90009', '422C0000', '4109999A', '3BA3D70A', 'BF7AE7B4', '00000000', '00000000', 'BE4B46FE'),
(3, 'Teletown - Al-Arqas', 'Al-Arqas', '8F58003B', '4337D9DB', '4270BB64', '411536F4', '3F3504F7', '00000000', '00000000', 'BF3504F7'),
(4, 'Teletown - Al-Jalima', 'Al-Jalima', '8588002C', '42F0B7CF', '42BEF0A4', '42B4192C', '3F800000', '00000000', '00000000', '00000000'),
(5, 'Teletown - Arwic', 'Arwic', 'C6A90009', '423B3852', '4087020C', '4228051F', '3F800000', '00000000', '00000000', '00000000'),
(6, 'Teletown - Ayan Baqur', 'Ayan Baqur', '11340025', '42C79EE7', '42D7D1C7', '4228051F', '3F3603A3', '00000000', '00000000', 'BF3404DA'),
(7, 'Teletown - Baishi', 'Baishi', 'CE410007', '4149999A', '4318CCCD', '425C3852', 'BF0B6D76', '00000000', '00000000', 'BF56B325'),
(8, 'Teletown - Bandit Castle', 'Bandit Castle', 'BDD00006', '41873333', '42F10000', '42E63333', '3F3504F7', '00000000', '00000000', 'BF3504F7'),
(9, 'Teletown - Beach Fort', 'Beach Fort', '42DE000C', '41C80000', '42A90000', '3BA3D70A', 'BF2E976C', '00000000', '00000000', 'BF3B3A04'),
(10, 'Teletown - Bluespire', 'Bluespire', '21B00017', '4240C28F', '4325E3D7', '3BA3D70A', 'BDAB3F64', '00000000', '00000000', 'BF7F1A7E'),
(11, 'Teletown - Candeth Keep', 'Candeth Keep', '2B11003D', '433D2358', '42C59A27', '4240051F', 'BF6DE29B', '00000000', '00000000', 'BEBD2C38'),
(12, 'Teletown - Cragstone', 'Cragstone', 'BB9F0040', '43295BA6', '43284042', '4258051F', '3F142492', '00000000', '00000000', 'BF50C804'),
(13, 'Teletown - Crater Lake Village', 'Mt Esper-Crater Village', '90D00107', '42BF0AC1', '42A80000', '438A9A3D', 'BF3504F7', '00000000', '00000000', 'BF3504F7'),
(14, 'Teletown - Danby\'s Outpost', 'Danby\'s Outpost', '5A9C0004', '41BC0000', '429A3333', '40C028F6', '00000000', '00000000', '00000000', 'BF800000'),
(15, 'Teletown - Dryreach', 'Dryreach', 'DB75003B', '433A0000', '42820000', '42100000', 'BF400000', '00000000', '00000000', '3F266666'),
(16, 'Teletown - Eastham', 'Eastham', 'CE940035', '43170D91', '42E13852', '418B5687', 'BF6FC383', '00000000', '00000000', 'BEB36FA0'),
(17, 'Teletown - Fort Tethana', 'Fort Tethana', '2681001D', '429B6666', '42D83333', '43700147', 'BF05C26E', '00000000', '00000000', 'BF5A469D'),
(18, 'Teletown - Glenden Wood', 'Glenden Wood', 'A0A40025', '42C09AA0', '42EFB1AA', '426FD194', '3F3504F7', '00000000', '00000000', 'BF3504F7'),
(19, 'Teletown - Greenspire', 'Greenspire', '2BB5003C', '4332F53F', '42AD23D7', '3BA3D70A', '3EB466F5', '00000000', '00000000', 'BF6F951C'),
(20, 'Teletown - Hebian-to', 'Hebian-to', 'E64E002F', '430A4DD3', '4321E7AE', '41A05194', '3F6C8366', '00000000', '00000000', 'BEC3EF07'),
(21, 'Teletown - Holtburg', 'Holtburg', 'A9B40019', '42A80000', '40E33333', '42BC0290', '3F7F35F4', '00000000', '00000000', 'BDA0AF1D'),
(22, 'Teletown - Kara', 'Kara', 'BA170039', '43353333', '404CCCCD', '43279AE1', 'BF5919AC', '00000000', '00000000', 'BF07A8C6'),
(23, 'Teletown - Khayyaban', 'Khayyaban', '9F44001A', '42B40000', '41C46C8B', '4212344F', 'BF4858FF', '00000000', '00000000', 'BF1F5D25'),
(24, 'Teletown - Kryst', 'Kryst', 'E822002A', '4304B333', '4217999A', '41A0D70B', 'BF5DB3D0', '00000000', '00000000', 'BF000000'),
(25, 'Teletown - Lin', 'Lin', 'DC3C0011', '426EE148', '412C624E', '419069D0', 'BEB77C03', '00000000', '00000000', 'BF6EFF19'),
(26, 'Teletown - Linvak Tukal', 'Linvak Tukal', 'A21E001A', '42A60000', '42180000', '440C1733', '3F800000', '00000000', '00000000', '00000000'),
(27, 'Teletown - Lytelthorpe', 'Lytelthorpe', 'C0800007', '413B9168', '431B8F5C', '42041CC2', 'BECE0286', '00000000', '00000000', 'BF6A5CE6'),
(28, 'Teletown - MacNiall\'s Freehold', 'MacNiall\'s Freehold', 'F224001A', '42A3999A', '42040000', '3BA3D70A', '3E76DC5D', '00000000', '00000000', 'BF787326'),
(29, 'Teletown - Mayoi', 'Mayoi', 'E6320021', '42D6D581', '412C353F', '41EF433E', 'BF248DC1', '00000000', '00000000', 'BF441B76'),
(30, 'Teletown - Nanto', 'Nanto', 'E63E0022', '42C1EB85', '4216E354', '42952625', '00000000', '00000000', '00000000', 'BF800000'),
(31, 'Teletown - Neydisa', 'Neydisa', '95D60033', '4312E666', '428E999A', '42C786D4', 'BF3B3A04', '00000000', '00000000', 'BF2E976C'),
(32, 'Teletown - Oolutanga\'s Refuge', 'Oolutanga\'s Refuge', 'F6820033', '4311B333', '42476B85', '4268051F', 'BEEF61ED', '00000000', '00000000', 'BF624BDC'),
(33, 'Teletown - Plateau Village', 'Plateau Village', '49B70021', '42C83333', '41A66666', '436E9D03', 'BF167914', '00000000', '00000000', 'BF4F1BBD'),
(34, 'Teletown - Qalaba\'r', 'Qalaba\'r', '9722003A', '43285AA0', '41C4F1AA', '42CC0290', 'BF6C3BF7', '00000000', '00000000', 'BEC5464E'),
(35, 'Teletown - Redspire', 'Redspire', '17B2002A', '43049F7D', '41CE78D5', '4230051F', '3F7F9C95', '00000000', '00000000', 'BD6189BE'),
(36, 'Teletown - Rithwic', 'Rithwic', 'C98C0028', '42E354CA', '433E424E', '41B00A3D', 'BF3504F7', '00000000', '00000000', 'BF3504F7'),
(37, 'Teletown - Samsur', 'Samsur', '977B000C', '41CE7CEE', '4293B4BC', '3BA3D70A', '3F6E1134', '00000000', '00000000', 'BEBC4157'),
(38, 'Teletown - Sawato', 'Sawato', 'C95B0001', '416CCCCD', '3E99999A', '4140147A', '3F6E2FE0', '00000000', '00000000', 'BEBBA605'),
(39, 'Teletown - Shoushi', 'Shoushi', 'DA55001D', '42A9999A', '42C60000', '41A00A3D', '3F800000', '00000000', '00000000', '00000000'),
(40, 'Teletown - Singularity Caul', 'Singularity Caul Island', '09040008', '41366666', '433C999A', '42AF0B18', 'BF7F1077', '00000000', '00000000', 'BDAEEF1C'),
(41, 'Teletown - Stonehold', 'Stonehold', '64D5000B', '41F00000', '42480000', '429C0290', '3F57E879', '00000000', '00000000', 'BF098C7E'),
(42, 'Teletown - Timaru', 'Timaru', '1DB60025', '42C50000', '42C43333', '42F00290', '3F4F5EEA', '00000000', '00000000', 'BF161C7A'),
(43, 'Teletown - Tou-Tou', 'Tou-Tou', 'F65C002B', '42FCC625', '42589687', '41A00A3D', '3F6DBBAE', '00000000', '00000000', 'BEBDEFE9'),
(44, 'Teletown - Tufa', 'Tufa', '876C0008', '40000000', '433AE666', '41900A3D', 'BF3504F7', '00000000', '00000000', 'BF3504F7'),
(45, 'Teletown - Underground City', 'Underground City', '01E901AD', '42F00000', 'C3020000', 'C13FEB86', 'BF36E47E', '00000000', '00000000', 'BF332064'),
(46, 'Teletown - Uziz', 'Uziz', 'A260003C', '4336EB44', '42AFDE35', '41A00A3D', 'BEBA17D3', '00000000', '00000000', 'BF6E7DED'),
(47, 'Teletown - Wai Jhou', 'Wai Jhou', '3F31001F', '42A6334D', '431C310A', '4031EC53', '3F0A3AD2', '00000000', '00000000', '3F577910'),
(48, 'Teletown - Xarabydun', 'Xarabydun', '934B0021', '42D899D2', '40C32B22', '4191273D', 'BF76ED35', '00000000', '00000000', 'BE871A5D'),
(49, 'Teletown - Yanshi', 'Yanshi', 'B46F001E', '42966666', '42F83333', '420AC0DB', '3F800000', '00000000', '00000000', '00000000'),
(50, 'Teletown - Yaraq', 'Yaraq', '7D64000D', '41FF3333', '42D13333', '413F258C', '3F13BFC6', '00000000', '00000000', 'BF510F73'),
(51, 'Teletown - Zaikhal', 'Zaikhal', '80900013', '4281B9DB', '425EBF7D', '42F80290', 'BF6E0CBF', '00000000', '00000000', 'BEBC57C1');

-- --------------------------------------------------------

--
-- Table structure for table `weenies`
--

CREATE TABLE `weenies` (
  `id` int(11) UNSIGNED NOT NULL,
  `top_level_object_id` int(11) UNSIGNED NOT NULL,
  `block_id` int(11) UNSIGNED NOT NULL,
  `data` mediumblob NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `accounts`
--
ALTER TABLE `accounts`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `username` (`username`);

--
-- Indexes for table `blocks`
--
ALTER TABLE `blocks`
  ADD PRIMARY KEY (`weenie_id`),
  ADD KEY `block_id` (`block_id`);

--
-- Indexes for table `characters`
--
ALTER TABLE `characters`
  ADD UNIQUE KEY `weenie_id` (`weenie_id`),
  ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `globals`
--
ALTER TABLE `globals`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `houses`
--
ALTER TABLE `houses`
  ADD PRIMARY KEY (`house_id`);

--
-- Indexes for table `teletowns`
--
ALTER TABLE `teletowns`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `weenies`
--
ALTER TABLE `weenies`
  ADD PRIMARY KEY (`id`),
  ADD KEY `top_level_object_id` (`top_level_object_id`),
  ADD KEY `block_id` (`block_id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `accounts`
--
ALTER TABLE `accounts`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=957;
--
-- AUTO_INCREMENT for table `globals`
--
ALTER TABLE `globals`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;
--
-- AUTO_INCREMENT for table `houses`
--
ALTER TABLE `houses`
  MODIFY `house_id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `teletowns`
--
ALTER TABLE `teletowns`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=52;
--
-- AUTO_INCREMENT for table `weenies`
--
ALTER TABLE `weenies`
  MODIFY `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2147483647;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
