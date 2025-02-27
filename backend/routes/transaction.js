const express = require('express');
const router = express.Router();
const transaction = require('../models/transaction_model');

// Get all
router.get('/', function(req, res) {
  transaction.getAll(function(err, result) {
    if (err) {
      res.status(500).json(err);
    } else {
      res.json(result);
    }
  });
});

// Get all by account
router.get('/account/:idaccount', function(req, res) {
  transaction.getByAccountId(req.params.idaccount, function(err, result) {
    if (err) {
      res.status(500).json(err);
    } else {
      res.json(result);
    }
  });
});

// Get one by transaction ID
router.get('/transaction/:idtransaction', function(req, res) {
  transaction.getByTransactionId(req.params.idtransaction, function(err, result) {
    if (err) {
      res.status(500).json(err);
    } else {
      res.json(result);
    }
  });
});

//Create transaction
router.post('/', function(req, res) {
  transaction.add(req.body, function(err, result) {
    if (err) {
      res.status(500).json(err);
    } else {
      res.status(201).json({ message: 'Transaction added', id: result.insertId });
    }
  });
});

// Update transaction descript
router.put('/:idtransaction', function(req, res) {
  transaction.update(req.params.idtransaction, req.body.description, function(err, result) {
    if (err) {
      res.status(500).json(err);
    } else {
      res.json({ message: 'Transaction updated' });
    }
  });
});

// Delete transaction
router.delete('/:id', function(req, res) {
  transaction.delete(req.params.id, function(err, result) {
    if (err) {
      res.status(500).json(err);
    } else {
      res.json({ message: 'Transaction deleted' });
    }
  });
});

module.exports = router;