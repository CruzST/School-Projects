-- for the store owner page
-- select the items and get the rating in the inventory 
SELECT name,quantity,price, ratings 
FROM inventory i 
JOIN rating r 
ON i.productid = r.productid;

--insert new item 
INSERT INTO inventory (name, quantity, price) 
VALUES (:name, :quantity, :price);

--update inventory
UPDATE inventory 
SET(name = :nameInput, quantity = :quantityInput, price = :priceInput);

--delete item from inventory
DELETE FROM inventory 
WHERE product_id = product_ID_selected_from_inventory_page;

--delete rating because of many to many relationship with inventory
DELETE FROM rating 
WHERE product_id = :product_id 
FROM inventory;

--purchase history page
SELECT accountid, concat(firstname, " ", lastname),productid, productname, p.quantity 
FROM purchases p 
JOIN customer c ON  p.accountid = c.accountid 
JOIN inventory i ON p.productid = i.productid;

--create a customer
INSERT INTO Customer (firstname, lastname, email, password, phonenumber, city, street, zip, state) 
VALUES (:firstname, :lastname, :email, :password, :phonenumber, :city, :street, :zip, :state);

--shopping page
--display list of items for customer shop
SELECT name,quantity,price 
FROM inventory i 
JOIN rating r 
ON i.product_id = r.product_id;

--search function for customer
SELECT product_id, name, quantity, price 
FROM inventory 
WHERE = :product_name;  -- where STRING = product name

--add purchases into table
INSERT INTO purchases(account_id, product_id, quantity) 
VALUES (:account_id 
	FROM customers table,:product_id 
	FROM inventory table,:quantity
	);

--add customer rating into table
INSERT INTO rating(account_id, product_id,rating) 
VALUES(:account_id 
	FROM custoemrs table,:product_id 
	FROM inventory table, rating
	);