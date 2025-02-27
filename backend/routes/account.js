const express = require('express');
const router = express.Router();
const accountModel = require('../models/account_model');

router.get('/', (req, res) => {
  accountModel.getAll((err, results) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json(results);
    }
  });
});

router.get('/:id', (req, res) => {
  const id = req.params.id;
  accountModel.getById(id, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json(result[0]);
    }
  });
});

router.post('/', (req, res) => {
  const newAccount = req.body;
  accountModel.add(newAccount, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.status(201).json({ message: 'Account added!', id: result.insertId });
    }
  });
});

router.put('/:id', (req, res) => {
  const id = req.params.id;
  const updatedAccount = req.body;
  accountModel.update(id, updatedAccount, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json({ message: 'Account updated!' });
    }
  });
});

router.delete('/:id', (req, res) => {
  const id = req.params.id;
  accountModel.delete(id, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json({ message: 'Account deleted!' });
    }
  });
});

module.exports = router;
