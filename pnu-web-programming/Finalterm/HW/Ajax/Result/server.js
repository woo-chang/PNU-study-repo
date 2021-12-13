const express = require('express');
const mysql = require('mysql')
const connection = mysql.createConnection({
    host: '127.0.0.1',
    user: 'root',
    password: '',
    database: '201724603'
})
const app = express();

//cors정책을 피하기 위한 모듈
const cors = require('cors')
// json 파일 받아서 사용하기 위한 모듈
var bodyParser = require('body-parser')

app.use(cors());
app.use(bodyParser.json())
app.use(bodyParser.urlencoded({extended:true}))

app.get('/', (req, res) => {
    res.send('hello')
})

app.post('/insert', function(req,res){
    var title = req.body.linkTitle;
    var src = req.body.linkSrc
    connection.query('SELECT * FROM mylinkTable WHERE linkTitle = "' + title + '"', function(error, result, fields) {
        if(error) {
            return res.json([{linkTitle: 'error', message: 'Fail to Load'}])
        } else {
            if(result.length >= 1) {
                return res.json([{linkTitle: 'error', message: '이미 있는 즐겨찾기 제목입니다. 다른 제목을 사용해주세요.'}])
            } else {
                connection.query('INSERT INTO mylinkTable(linkTitle, linkSrc) VALUES("' + title + '", "' + src + '")', function(error, result, fields) {
                    if(error) {
                        return res.json([{linkTitle: 'error', message: 'Fail to Insert'}])
                    } else {
                        return res.json([{linkTitle: 'Good', message: 'Good'}])
                    }
                })
            }
        }
    })
})

app.post('/list', function(req, res)  {
    connection.query('SELECT * FROM mylinkTable ORDER BY linkTitle', function(error, result, fields) {
        if(error) {
            return res.json([{linkTitle: 'error', message: 'Fail to Load'}])
        } else {
            return res.json(result)
        }
    })
})

app.post('/deleteAll', function(req, res) {
    connection.query('DELETE FROM mylinkTable', function(error, result, fields) {
        if(error) {
            return res.json([{linkTitle: 'error', message: 'Fail to Delete'}])
        } else {
            return res.json([{linkTitle: 'Good', message: 'Good'}])
        }
    })
})

app.post('/delete', function(req, res) {
    var title = req.body.linkTitle
    connection.query('SELECT * FROM mylinkTable WHERE linkTitle = "' + title + '"', function(error, result, fields) {
        if(error) {
            return res.json([{linkTitle: 'error', message: 'Fail to Load'}])
        } else {
            if(result.length >= 1) {
                connection.query('DELETE FROM mylinkTable WHERE linkTitle = "' + title + '"', function(error, result, fields) {
                    if(error) {
                        return res.json([{linkTitle: 'error', message: 'Fail to Delete'}])
                    } else {
                        return res.json([{linkTitle: 'Good', message: 'Good'}])
                    }
                })
            } else {
                return res.json([{linkTitle: 'error', message: '일치하는 제목을 가진 즐겨찾기가 없습니다.'}])
            }
        }
    })
    
})

app.listen(3000, () => {
    console.log('listen to 3000')
})