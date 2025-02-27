const db = require('../database');

const customer = {
    getAll: (callback) => {
        db.query("SELECT * FROM customer", callback);
    },

    getById: (id, callback) => {
        db.query("SELECT * FROM customer WHERE idcustomer = ?", [id], callback);
    },

    add: (customerData, callback) => {
        const { fname, lname, thumbnail } = customerData;
        db.query(
            "INSERT INTO customer (fname, lname, thumbnail) VALUES (?, ?, ?)",
            [fname, lname, thumbnail],
            callback
        );
    },

    update: (id, customerData, callback) => {
        const { fname, lname, thumbnail } = customerData;
        db.query(
            "UPDATE customer SET fname = ?, lname = ?, thumbnail = ? WHERE idcustomer = ?",
            [fname, lname, thumbnail, id],
            callback
        );
    },

    delete: (id, callback) => {
        db.query("DELETE FROM customer WHERE idcustomer = ?", [id], callback);
    },

    getThumbnailByUserId: (userId, callback) => {
        db.query(
            "SELECT thumbnail FROM customer WHERE idcustomer = ?",
            [userId],
            callback
        );
    },

    updateThumbnail: (userId, fileName, callback) => {
        db.query(
            "UPDATE customer SET thumbnail = ? WHERE idcustomer = ?",
            [fileName, userId],
            callback
        );
    }
};

module.exports = customer;
