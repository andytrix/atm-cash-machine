const express = require('express');
const router = express.Router();
const cardModel = require('../models/card_model');

router.get('/', (req, res) => {
  cardModel.getAll((err, results) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json(results);
    }
  });
});

router.get('/:id', (req, res) => {
  const id = req.params.id;
  cardModel.getById(id, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json(result[0]);
    }
  });
});

router.post('/', (req, res) => {
  const newCard = req.body;
  cardModel.add(newCard, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.status(201).json({ message: 'Card added!', id: result.insertId });
    }
  });
});

router.put('/:id', (req, res) => {
  const id = req.params.id;
  const updatedCard = req.body;
  cardModel.update(id, updatedCard, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json({ message: 'Card updated!' });
    }
  });
});

router.delete('/:id', (req, res) => {
  const id = req.params.id;
  cardModel.delete(id, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json({ message: 'Card deleted!' });
    }
  });
});

module.exports = router;