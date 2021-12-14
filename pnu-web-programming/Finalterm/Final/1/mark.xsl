<?xml version = "1.0"?>

<xsl:stylesheet version = "1.0"                       
    xmlns:xsl = "http://www.w3.org/1999/XSL/Transform">

    <xsl:output method = "html" doctype-system = "about:legacy-compat" />

    <xsl:template match = "/">  
        <html>
            <xsl:apply-templates/>
        </html>
    </xsl:template>
          
    <xsl:template match = "mark">
        <head>
            <meta charset = "utf-8"/>
            <link rel = "stylesheet" type = "text/css" href = "style.css"/>
            <title>즐겨찾기 정보</title>    
        </head>

        <body>

            <div id="id1">
                <img>
                    <xsl:attribute name = "src">
                    <xsl:value-of select = "image"/>
                    </xsl:attribute>
                </img>

                <div id="id3">
                    <p>
                        [<xsl:value-of select = "classify"/>]
                    </p>

                    <a>
                        <xsl:attribute name = "href">
                        <xsl:value-of select = "link"/>
                        </xsl:attribute>
                        <xsl:value-of select = "name"/>
                    </a>

                    <p>
                        (<xsl:value-of select = "count"/>회 방문)
                    </p>
                </div>
            </div>
            
        </body>
    </xsl:template>
</xsl:stylesheet>