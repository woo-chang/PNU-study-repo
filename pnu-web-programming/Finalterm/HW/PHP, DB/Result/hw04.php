<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <title>HW04</title>
    <style type="text/css">
        #linkContainer {
            display: flex;
            flex-wrap: wrap;
            width: 310px;
        }
        #linkContainer a {
            display: block;
            list-style-type: none;
            text-align: center;
            font-size: 3em;
            border: 1px solid;
            width: 150px;
            float: left;
            margin: 1px;
        }        
        a { 
            text-decoration: none;
        }
        .pagination {
            display: inline-block;
        }
        /* submit으로 동작해야되기 때문에 div -> input으로 수정 */
        .pagination input {
            float: left;
            padding: 8px 16px;
        }
        .pagination input.active {
            background-color: #4CAF50;
            color: white;
        }
        .pagination input:hover:not(.active) {background-color: #ddd;}
        .maindiv {
            width: 320px;
            float: left;
        }
        iframe {
            width: 500px;
            height: 500px;
            float: left;
        }
    </style>
    </head>
<body>
    <div class="maindiv">
        <form name="form1" method="GET" action="hw04.php">
            <label>즐겨찾기 제목: <input type="text" name="linkTitle" id="linkTitle"></label><br>
            <label>즐겨찾기 링크: <input type="text" name="linkSrc" id="linkSrc"></label><br>
            <!-- 자바스크립트와 다르게 submit으로 동작해야 함 -->
            <input type="submit" name="addBtn" id="addBtn" value="즐겨찾기 추가">
            <input type="submit" name="delBtn" id="delBtn" value="즐겨찾기 삭제">
            <input type="submit" name="clearBtn" id="clearBtn" value="모두 삭제">  
        </form>
        <br>
        <div id="linkContainer">
        <?php
        extract($_GET); // 변수처럼 쉽게 사용하기 위해 사용
        
        $mysqli = new mysqli("localhost", "root", "woo7ang", "201724603");
        if($mysqli->connect_errno){
            echo "Failed to connect to MySQL";
        } // sql connect

        $curPage = 0; // 현재 페이지를 나타내는 변수

        // 링크를 추가하는 함수
        function addLink() {
            extract($_GET); // 변수처럼 쉽게 사용하기 위해 사용
            global $mysqli; // 함수 내에서 사용할 전역 변수 명시
            
            // 둘 중 하나라도 입력되지 않으면 경고창 알리고 리턴
            if ($linkTitle == "" || $linkSrc == "") {
                echo "<script>window.alert('추가하기 위해서는 입력이 필요합니다.');</script>";  
                return;
            }
            
            // select문을 사용해서 linkTitle이 있는 것을 가져옴
            if ($linkArr = $mysqli->query("select * from mylinktable where linkTitle = '$linkTitle'")) {
                // 존재하면 이미 있는 즐겨찾기 제목
                if ($linkArr->num_rows > 0) {
                    echo "<script>window.alert('이미 있는 즐겨찾기 제목입니다. 다른 제목을 사용해주세요.');</script>";  
                }
                else {
                    if ($mysqli->query("insert into mylinktable set linkTitle ='".$linkTitle."', linkSrc = '".$linkSrc."'") !== TRUE){
                        echo "Failed to insert into MySQl";
                    }
                }
            }
            // 자바스크립트를 조작
            echo "<script>document.forms[0].reset();</script>";
        }

        // 링크 1개를 지정해서 삭제하는 함수
        function deleteLink() {
            extract($_GET); // 변수처럼 쉽게 사용하기 위해 사용
            global $mysqli; // 함수 내에서 사용할 전역 변수 명시
            
            if ($linkArr = $mysqli->query("select * from mylinktable where linkTitle = '$linkTitle'")) {
                // 해당 즐겨찾기 제목이 존재하면 삭제, 없으면 경고창 표시
                if ($linkArr->num_rows > 0) {
                    $mysqli->query("delete from mylinktable where linkTitle = '$linkTitle'");
                } else {
                    echo "<script>window.alert('일치하는 제목을 가진 즐겨찾기가 없습니다.');</script>";
                }
            }
        }

        // 모든 링크를 삭제하는 함수
        function clearAll() {
            extract($_GET); // 변수처럼 쉽게 사용하기 위해 사용
            global $mysqli; // 함수 내에서 사용할 전역 변수 명시

            // 테이블 record 전부 삭제!
            if ($result = $mysqli->query("delete from mylinktable")) {
                echo "<script>window.alert('데이터베이스가 초기화 되었습니다.');</script>";  
            }
        }

        // 페이지에 알맞은 링크 리스트 목록을 보여주는 함수
        function showLinks() {
            extract($_GET); // 변수처럼 쉽게 사용하기 위해 사용
            global $mysqli; // 함수 내에서 사용할 전역 변수 명시
            global $curPage; // 함수 내에서 사용할 전역 변수 명시

            $startIdx = $curPage * 10;
            $endIdx = $startIdx + 9;
            // 개수를 지정하고 정렬해서 가져오는 query
            if ($linkArr = $mysqli->query("select * from mylinktable order by linkTitle asc limit $startIdx, 10")) {
                // 테이블에서 가져온 record를 하나씩 가져와서 처리할 수 있는 함수
                while ($record = mysqli_fetch_array($linkArr)) {
                    $link = makeLink($record['linkTitle'], "https://".$record['linkSrc']);
                    echo $link;
                }
            }
        }

        // 링크를 위한 앵커 태그를 생성하는 함수
        function makeLink($title, $src) {
            return "<a href='$src' target=\"linkFrame\">$title</a>";
        }

        // 링크가 10개를 초과하면 페이지 버튼을 생성하는 함수
        function makePages() {
            extract($_GET); // 변수처럼 쉽게 사용하기 위해 사용
            global $mysqli; // 함수 내에서 사용할 전역 변수 명시
            global $curPage; // 함수 내에서 사용할 전역 변수 명시

            $linkArr = $mysqli->query("select * from mylinktable");
            $linkCount = $linkArr->num_rows;
            $nPage = ceil($linkCount / 10);

            // 오류인지 모르겠으나 for문이 동작하게 되어 1이 항상 보이게 되길래 if문 추가
            if ($linkCount > 10) {
                for($i = 0; $i < $nPage; $i++) {
                    $c = $i + 1; // 0부터 시작하기에 시각적으로 1로 보이기위한 플러스
                    if ($i == $curPage) {
                        echo "<input class=\"active\" type=\"submit\" id=\"pageBtn\" name=\"pageBtn\" value=$c>";
                    } else {
                        echo "<input type=\"submit\" id=\"pageBtn\" name=\"pageBtn\" value=$c>";
                    }
                }
            }
        }

        // 두번째 파라미터인 배열에 해당 key가 존재하는지 확인하는 함수
        if (array_key_exists("addBtn", $_GET)) {
            addLink(); // 링크 추가하는 함수 실행
        }

        if (array_key_exists("delBtn",$_GET)) {
            deleteLink(); // 링크 삭제하는 함수 실행
        }

        if (array_key_exists("clearBtn",$_GET)) {
            clearAll(); // 초기화 하는 함수 실행
        }

        if (array_key_exists("pageBtn",$_GET)) {
            $curPage = $pageBtn - 1; // 시각적으로 보이는 요소 제거하기 위한 마이너스
        }

        showLinks(); // 동작할 때 마다 링크가 보이는 것을 갱신해주기 위한 함수 실행
        
        ?>
        </div>
        <form class="pagination" name="pagNav" method="GET" action="hw04.php">
            <?php
            makePages(); // 링크 개수에 따라 페이지 버튼을 갱신해주기 위한 함수 실행
            ?>
        </form>
        
    </div>
    <iframe title="iframe for link" name="linkFrame"></iframe>
</body>
</html>