const dotenv=require('dotenv');
const mysql = require('mysql2');

dotenv.config();

const connection = mysql.createPool({
    host: process.env.DB_HOST || '127.0.0.1',
    user: process.env.DB_USER || 'bankuser',
    password: process.env.DB_PASS || 'bankuser',
    database: process.env.DB_NAME || 'bank-automat',
    waitForConnections: true,
    connectionLimit: 10,
    queueLimit: 0
  });

module.exports=connection;




