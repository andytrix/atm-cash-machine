const express = require('express');
const router = express.Router();
const bcrypt = require('bcryptjs');
const card = require('../models/card_model');
const jwt = require('jsonwebtoken');
const dotenv = require('dotenv');
const db = require('../database');

let pinAttempts = {};

function lockCardInDB(idcard, callback) {
  const sql = "UPDATE card SET status = 'locked' WHERE idcard = ?";
  db.query(sql, [idcard], (err, result) => {
    if (err) return callback(err);
    callback(null); 
  });
}

router.post('/', (request, response) => {
  const cardID = request.body.idcard;
  const cardPIN = request.body.pin;

  if (!cardID || !cardPIN) {
    console.log("Card ID or PIN missing");
    return response.send("false");
  }

  card.checkPin(cardID, (dbError, dbResult) => {
    if (dbError) {
      console.log("DB error:", dbError);
      return response.send("!404!");
    }

    if (dbResult.length > 0) {
      const row = dbResult[0];

      if (row.status === 'locked') {
        console.log("Card is locked in DB");
        return response.send("locked");
      }

      bcrypt.compare(cardPIN, row.pin, (err, compareResult) => {
        if (err) {
          console.log("bcrypt error:", err);
          return response.send("!404!");
        }

        if (compareResult) {
          pinAttempts[cardID] = 0;
          dotenv.config();
          const token = jwt.sign({ idcard: cardID }, process.env.MY_TOKEN, { expiresIn: '1800s' });
          console.log("Success -> token returned");
          return response.send(token);

        } else {
          if (!pinAttempts[cardID]) {
            pinAttempts[cardID] = 0;
          }
          pinAttempts[cardID]++;

          console.log("Wrong PIN for card:", cardID, ", attempts:", pinAttempts[cardID]);

          if (pinAttempts[cardID] >= 3) {
            lockCardInDB(cardID, (lockErr) => {
              if (lockErr) {
                console.log("Lock card error:", lockErr);
                return response.send("!404!");
              }
              console.log("Card locked in DB");
              return response.send("locked");
            });
          } else {
            return response.send("false");
          }
        }
      });
    } else {
      console.log("Card does not exist");
      return response.send("false");
    }
  });
});

module.exports = router;
