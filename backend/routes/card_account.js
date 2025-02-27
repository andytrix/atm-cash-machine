const express = require('express');
const router = express.Router();
const cardAccountModel = require('../models/card_account_model');

router.get('/', (req, res) => {
  cardAccountModel.getAll((err, results) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json(results);
    }
  });
});

router.get('/:id', (req, res) => {
  const id = req.params.id;
  cardAccountModel.getById(id, (err, results) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json(results[0]);
    }
  });
});

router.post('/', (req, res) => {
  const newCardAccount = req.body;
  cardAccountModel.add(newCardAccount, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.status(201).json({ message: 'Card account added!', id: result.insertId });
    }
  });
});

router.put('/:id', (req, res) => {
  const id = req.params.id;
  const updatedCardAccount = req.body;
  cardAccountModel.update(id, updatedCardAccount, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json({ message: 'Card account updated!' });
    }
  });
});

router.delete('/:id', (req, res) => {
  const id = req.params.id;
  cardAccountModel.delete(id, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json({ message: 'Card account deleted!' });
    }
  });
});

module.exports = router;
