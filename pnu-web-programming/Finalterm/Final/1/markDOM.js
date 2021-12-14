var outputHTML = "";
var idCounter = 1;
var firstText = false;
var myName = ''; 

function start() {
  loadXMLDocument("mark.xml");
}

// xml 문서 가져오는 함수
function loadXMLDocument(url) {
  var xmlHttpRequest = new XMLHttpRequest(); // 외부 파일 가져올 때 사용
  xmlHttpRequest.open("get", url, false);
  xmlHttpRequest.send(null);
  var doc = xmlHttpRequest.responseXML;
  buildHTML(doc.childNodes);
  displayDoc();
}

function buildHTML(childList) {
  for (var i = 0; i < childList.length; i++) {
    switch (childList[i].nodeType) {
      case 1:
        if (childList[i].nodeName == "image") {
          outputHTML +=
            '<img id="id' +
            idCounter +
            '" src="' +
            childList[i].childNodes[0].nodeValue +
            '"/>';
          ++idCounter;
          firstText = true;
        } else {
          if (firstText) {
            outputHTML += '<div id="id' + idCounter + '">';
            ++idCounter;
            firstText = false;
          }

          if (childList[i].nodeName == "name") {
            outputHTML += '<span id="id' + idCounter + '">';
            
            //outputHTML += childList[ i ].nodeName; // show node's name
            ++idCounter;

            if (childList[i].childNodes.length != 0){
                myName = childList[i].childNodes[0].nodeValue;
            }
            
            outputHTML += " ";
          } else if (childList[i].nodeName == "link") {
            //outputHTML += childList[ i ].nodeName; // show node's name

            if (childList[i].childNodes.length != 0){
                outputHTML += '<a href="' + childList[i].childNodes[0].nodeValue + '">' + myName + '</a>'
            }
            outputHTML += "</span>";
          } else if (childList[i].nodeName == 'classify') {
            outputHTML += '<span id="id' + idCounter + '">';

            //outputHTML += childList[ i ].nodeName; // show node's name
            ++idCounter;

            if (childList[i].childNodes.length != 0) {
                outputHTML += "[" + childList[i].childNodes[0].nodeValue + "]"
            }
            
            outputHTML += "</span>";
          } else if (childList[i].nodeName == 'count'){
            outputHTML += '<span id="id' + idCounter + '">';

            //outputHTML += childList[ i ].nodeName; // show node's name
            ++idCounter;

            if (childList[i].childNodes.length != 0) {
                outputHTML += "(" + childList[i].childNodes[0].nodeValue + "회 방문)"
            }

            outputHTML += "</span>";
          } else {
            outputHTML += '<span id="id' + idCounter + '">';

            //outputHTML += childList[ i ].nodeName; // show node's name
            ++idCounter;

            if (childList[i].childNodes.length != 0)
              buildHTML(childList[i].childNodes);
            outputHTML += "</span>";
          }
        }
        break;
      case 3:
      case 8:
        if (
          childList[i].nodeValue.indexOf("   ") == -1 &&
          childList[i].nodeValue.indexOf("      ") == -1
        ) {
          outputHTML += '<span id="id' + idCounter + '">';

          outputHTML += childList[i].nodeValue + "</span>";
          ++idCounter;
        }
    }
  }
}

function displayDoc() {
  outputHTML += "</div>";
  document.getElementById("outputDiv").innerHTML = outputHTML;
}

window.addEventListener("load", start, false);