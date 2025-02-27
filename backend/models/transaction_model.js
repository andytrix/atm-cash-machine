const db = require('../database');

const transaction = {
  getAll: function(callback) {
    return db.query('SELECT * FROM transaction', callback);
  },
  getByTransactionId: function(idtransaction, callback) {
    return db.query('SELECT * FROM transaction WHERE idtransaction=?', [idtransaction], callback);
  },
  getByAccountId: function(idaccount, callback) {
    return db.query('SELECT * FROM transaction WHERE idaccount=?', [idaccount], callback);
  },
  add: function(transaction_data, callback) {
    return db.query(
      'INSERT INTO transaction (idaccount, type, amount, description) VALUES (?, ?, ?, ?)',
      [transaction_data.idaccount, transaction_data.type, transaction_data.amount, transaction_data.description],
      callback
    );
  },
  update: function(idtransaction, description, callback) {
    return db.query(
      'UPDATE transaction SET description=? WHERE idtransaction=?',
      [description, idtransaction],
      callback
    );
  },
  delete: function(id, callback) {
    return db.query('DELETE FROM transaction WHERE idtransaction=?', [id], callback);
  }
};

module.exports = transaction;