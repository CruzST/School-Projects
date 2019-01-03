DROP TABLE IF EXISTS `purchases`;
DROP TABLE IF EXISTS `rating`;
DROP TABLE IF EXISTS `customer`;
DROP TABLE IF EXISTS `inventory`;


CREATE TABLE customer(
	account_id INT AUTO_INCREMENT PRIMARY KEY,
	first_name VARCHAR(255) NOT NULL,
	last_name VARCHAR(255) NOT NULL,
  email VARCHAR(255) NOT NULL,
  password VARCHAR(255) NOT NULL,
  street VARCHAR(255),
  state VARCHAR(255),
  city VARCHAR(255),
  zip	VARCHAR(5),
  phone_number VARCHAR(255)
) ENGINE = InnoDB;

CREATE TABLE inventory(
	product_id INT AUTO_INCREMENT PRIMARY KEY,
  name VARCHAR(255),
	quantity INT,
  price DECIMAL (6,2)
) ENGINE = InnoDB;

CREATE TABLE purchases(
	order_id INT AUTO_INCREMENT PRIMARY KEY,
  account_id INT(11)NOT NULL DEFAULT '0', 
	product_id INT(11)NOT NULL DEFAULT '0',
  quantity int(11),
	FOREIGN KEY (account_id) REFERENCES customer(account_id) ON DELETE CASCADE ON UPDATE CASCADE,
  FOREIGN KEY (product_id) REFERENCES inventory(product_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE = InnoDB;

CREATE TABLE rating(
	rating_id INT AUTO_INCREMENT PRIMARY KEY,
	`product_id` INT(11) NOT NULL DEFAULT '0',
  	ratings int(11),
  	FOREIGN KEY (`product_id`) REFERENCES inventory(`product_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE = InnoDB;
  	


-- SAMPLE DATA FOR TABLES
LOCK TABLES `customer` WRITE;
INSERT INTO `customer` VALUES
	(1, "goblin", "slayer", "goblinslayer@gmail.com", "goblinslayer" , "123 blacksmith way", "New Jersey", "goldshire", "07019", "444-555-6666"),
  (2, "sylvanas", "windrunner", "windrunners@gmail.com", "forthehorde", "456 rogue street", "azeroth", "undercity", "58482", "555-555-5555"),
  (3, "dwayne", "johnson", "therock@gmail.com", "wrestling", "123 hollywood street", "California", "hollywood", "19103", "999-999-9999"),
  (4, "lockon", "stratos", "gundam00@gmail.com", "setsuna", "123 fake street", "UC0204", "ireland", "30902", "123-456-7890"),
  (5, "Joseph", "Joestar", "jojos@hotmail.com", "diobrando", "456 N3rd Street", "New York City", "NewYork", "01042", "212-451-2312");
UNLOCK TABLES;


LOCK TABLES `inventory` WRITE;
INSERT INTO `inventory` VALUES
	(1, "sword of goblin slayer", 2 , 99.99),
  (2, "M200", 100, 499.99),
  (3, "anti goblin spray", 1000, 9.99),
  (4, "holy hand grenade", 142, 666.00),
  (5, "guild girl figurine", 3 , 79.99);
UNLOCK TABLES;

LOCK TABLES `purchases` WRITE;
INSERT INTO `purchases`	VALUES
(1, 1, 1, 1),
(2, 3, 2, 1),
(3, 1, 5, 2),
(4, 5, 4, 10),
(5, 2, 3, 3);
UNLOCK TABLES;


LOCK TABLES `rating` WRITE;
INSERT INTO `rating` VALUES
  (1,1,10),
  (2,2,9),
  (3,5,10),
  (4,4,7),
  (5,5,5);
UNLOCK TABLES;
