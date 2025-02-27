const express = require('express');
const router = express.Router();
const customerModel = require('../models/customer_model');
const path = require('path');
const fs = require('fs');
const multer = require('multer');

const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, path.join(__dirname, '../public/images'));
  },
  filename: (req, file, cb) => {
    cb(null, Date.now() + path.extname(file.originalname));
  }
});

const upload = multer({ storage });

router.get('/', (req, res) => {
  customerModel.getAll((err, results) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json(results);
    }
  });
});

router.get('/getThumbnail', (req, res) => {
  const userId = req.query.userId;

  if (!userId) {
    return res.status(400).json({ error: 'userId puuttuu' });
  }

  customerModel.getThumbnailByUserId(userId, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else if (result.length === 0) {
      res.status(404).json({ error: 'Thumbnailia ei löytynyt' });
    } else {
      const filePath = path.join(__dirname, '../public/images', result[0].thumbnail);
      if (fs.existsSync(filePath)) {
        res.sendFile(filePath);
      } else {
        res.status(404).json({ error: 'Tiedostoa ei löydy palvelimelta' });
      }
    }
  });
});

router.post('/thumbnail', upload.single('thumbnail'), async (req, res) => {
    try {
        if (!req.file) {
            return res.status(400).json({ error: 'Tiedosto puuttuu' });
        }

        const userId = req.body.userId;
        if (!userId) {
            return res.status(400).json({ error: 'userId puuttuu' });
        }

        // Hae käyttäjän vanha kuva tietokannasta
        customerModel.getThumbnailByUserId(userId, async (err, result) => {
            if (err) {
                return res.status(500).json({ error: err.message });
            }

            if (result.length > 0 && result[0].thumbnail) {
                const oldFilePath = path.join(__dirname, '../public/images', result[0].thumbnail);
                
                // Poista vanha kuva jos se on olemassa
                if (fs.existsSync(oldFilePath)) {
                    fs.unlinkSync(oldFilePath);
                }
            }

            // Puske uusi kuva tietokantaan
            const fileName = req.file.filename;
            customerModel.updateThumbnail(userId, fileName, (err) => {
                if (err) {
                    return res.status(500).json({ error: err.message });
                }
                res.json({ success: true, fileName });
            });
        });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

router.get('/:id', (req, res) => {
  const id = req.params.id;
  customerModel.getById(id, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json(result[0]);
    }
  });
});

router.post('/', (req, res) => {
  const newCustomer = req.body;
  customerModel.add(newCustomer, (err, result) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.status(201).json({ message: 'Customer added!', id: result.insertId });
    }
  });
});

router.put('/:id', (req, res) => {
  const id = req.params.id;
  const updatedCustomer = req.body;
  customerModel.update(id, updatedCustomer, (err) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json({ message: 'Customer updated!' });
    }
  });
});

router.delete('/:id', (req, res) => {
  const id = req.params.id;
  customerModel.delete(id, (err) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json({ message: 'Customer deleted!' });
    }
  });
});

module.exports = router;