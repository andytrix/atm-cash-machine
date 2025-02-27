const db = require('../database');

const account = {
  getAll: function (callback) {
    return db.query('SELECT * FROM account', callback);
  },

  getById: function (id, callback) {
    return db.query('SELECT * FROM account WHERE idaccount = ?', [id], callback);
  },

  add: function (accountData, callback) {
    const { idcustomer, type, credit_balance, credit_limit, debit_balance } = accountData;
    return db.query(
      'INSERT INTO account (idcustomer, type, credit_balance, credit_limit, debit_balance) VALUES (?, ?, ?, ?, ?)',
      [idcustomer, type, credit_balance, credit_limit, debit_balance],
      callback
    );
  },

  update: function (id, accountData, callback) {
    const { idcustomer, type, credit_balance, credit_limit, debit_balance } = accountData;
    return db.query(
      'UPDATE account SET idcustomer=?, type=?, credit_balance=?, credit_limit=?, debit_balance=? WHERE idaccount=?',
      [idcustomer, type, credit_balance, credit_limit, debit_balance, id],
      callback
    );
  },

  delete: function (id, callback) {
    return db.query('DELETE FROM account WHERE idaccount = ?', [id], callback);
  }
};

module.exports = account;
