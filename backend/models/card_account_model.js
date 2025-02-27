const db = require('../database');

const cardAccount = {
  getAll: function (callback) {
    return db.query('SELECT * FROM card_account', callback);
  },

  getById: function (id, callback) {
    return db.query('SELECT * FROM card_account WHERE idcard_account = ?', [id], callback);
  },

  add: function (cardAccountData, callback) {
    const { idcard, idaccount, type } = cardAccountData;
    return db.query(
      'INSERT INTO card_account (idcard, idaccount, type) VALUES (?, ?, ?)',
      [idcard, idaccount, type],
      callback
    );
  },

  update: function (id, cardAccountData, callback) {
    const { idcard, idaccount, type } = cardAccountData;
    return db.query(
      'UPDATE card_account SET idcard=?, idaccount=?, type=? WHERE idcard_account=?',
      [idcard, idaccount, type, id],
      callback
    );
  },

  delete: function (id, callback) {
    return db.query('DELETE FROM card_account WHERE idcard_account = ?', [id], callback);
  }
};

module.exports = cardAccount;
