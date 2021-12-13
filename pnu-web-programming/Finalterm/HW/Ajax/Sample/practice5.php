<?php
    //ini_set('display_errors', 1);   // 에러 메시지 출력 On

    $post_data = json_decode(file_get_contents('php://input'));
    if($post_data->mode == '')
        die('{"err": "정상적인 요청이 아닙니다."}');

    // Connect to MySQL
    $mysqli = new mysqli("localhost", "root", "123456", "202112345");
    if ( $mysqli->connect_errno )
        die('{"err": "DB에 연결할 수 없습니다."}');

    if ($post_data->mode == "select")
    {
        $sql = "select * from bookmarks order by title asc";
        $data_arr = array();
        $data_cnt = 0;
        if ( $result = $mysqli->query($sql) ) 
        {
            while($data = $result->fetch_assoc())
            {
                $data_arr[$data_cnt++] = array("title"=>$data['title'], "link"=>$data['link']);
            }
            $result->close();
            echo json_encode($data_arr);
        }
    }
    else if ($post_data->mode == "insert")
    {
        // 중복 검사 먼저 수행
        $sql = "select * from bookmarks where title='".$post_data->linkTitle."'";
        if ( $result = $mysqli->query($sql) ) 
        {
            if ($result->num_rows > 0)
            {
                echo '{"err": "이미 있는 즐겨찾기 제목입니다. 다른 제목을 사용해주세요."}';
            }
            else
            {
                $sql = "insert into bookmarks set title = '".$post_data->linkTitle."', link = '".$post_data->linkSrc."'";
                $mysqli->query($sql);
            }
            $result->close();
        }
    }
    else if ($post_data->mode == "delete")
    {
        // 존재 여부 검사 먼저 수행
        $sql = "select * from bookmarks where title='".$post_data->linkTitle."'";
        if ( $result = $mysqli->query($sql) ) 
        {
            if ($result->num_rows == 0)
            {
                echo '{"err": "일치하는 제목을 가진 즐겨찾기가 없습니다."}';
            }
            else
            {
                $sql = "delete from bookmarks where title = '".$post_data->linkTitle."'";
                $mysqli->query($sql);
            }
            $result->close();
        }
    }
    else if ($post_data->mode == "delete_all")
    {
        $sql = "delete from bookmarks";
        $mysqli->query($sql);
    }
    $mysqli->close();
?>