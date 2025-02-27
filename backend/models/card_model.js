const db = require('../database');
const bcrypt = require('bcryptjs');

const card = {
  getAll: function (callback) {
    return db.query('SELECT * FROM card', callback);
  },

  getById: function (id, callback) {
    return db.query('SELECT * FROM card WHERE idcard = ?', [id], callback);
  },

  add: function (cardData, callback) {
    const { idcustomer, pin, cardtype, status, expiry_date } = cardData;
    bcrypt.hash(pin, 10, function (err, hashedPin) {
      if (err) {
        return callback(err, null);
      }
      return db.query(
        'INSERT INTO card (idcustomer, pin, cardtype, status, expiry_date) VALUES (?, ?, ?, ?, ?)',
        [idcustomer, hashedPin, cardtype, status, expiry_date],
        callback
      );
    });
  },

  update: function (id, cardData, callback) {
    const { idcustomer, pin, cardtype, status, expiry_date } = cardData;
    bcrypt.hash(pin, 10, function (err, hashedPin) {
      if (err) {
        return callback(err, null);
      }
      return db.query(
        'UPDATE card SET idcustomer=?, pin=?, cardtype=?, status=?, expiry_date=? WHERE idcard=?',
        [idcustomer, hashedPin, cardtype, status, expiry_date, id],
        callback
      );
    });
  },

  delete: function (id, callback) {
    return db.query('DELETE FROM card WHERE idcard = ?', [id], callback);
  },

  checkPin: function(idcard, callback){
    return db.query("SELECT pin, status FROM card WHERE idcard = ?", [idcard], callback);
  },

};

module.exports = card;