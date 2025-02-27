const db = require('./database');
const cardAccountRouter = require('./routes/card_account');
const transactionRoutes = require('./routes/transaction');
const accountRouter = require('./routes/account');
const customerRouter = require('./routes/customer');
const loginRouter = require('./routes/login');

db.getConnection((err, connection) => {
  if (err) {
    console.error('Database connection failed:', err);
  } else {
    console.log('Database connection successful.');
    connection.release();
  }
});

var express = require('express');
var path = require('path');
var cookieParser = require('cookie-parser');
var logger = require('morgan');

var indexRouter = require('./routes/index');
var cardRouter = require('./routes/card');
const jwt = require('jsonwebtoken');

var app = express();

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

app.use('/', indexRouter);
app.use('/login', loginRouter);
app.use(authenticateToken);
app.use('/card', cardRouter);
app.use('/card_account', cardAccountRouter);
app.use('/transaction', transactionRoutes);
app.use('/account', accountRouter);
app.use('/customer', customerRouter);

function authenticateToken(req, res, next) {
  const authHeader = req.headers['authorization']
  const token = authHeader && authHeader.split(' ')[1]

  console.log("token = "+token);
  if (token == null) return res.sendStatus(401)

  jwt.verify(token, process.env.MY_TOKEN, function(err, user) {

    if (err) return res.sendStatus(403)

    req.user = user

    next()
  })
}


module.exports = app;
