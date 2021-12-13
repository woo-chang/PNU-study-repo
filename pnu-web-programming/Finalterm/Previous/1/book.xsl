<?xml version = "1.0"?>

<xsl:stylesheet version = "1.0"                       
    xmlns:xsl = "http://www.w3.org/1999/XSL/Transform">

    <xsl:output method = "html" doctype-system = "about:legacy-compat" />

    <xsl:template match = "/">  
        <html>
            <xsl:apply-templates/>
        </html>
    </xsl:template>
          
    <xsl:template match = "book">
        <head>
            <meta charset = "utf-8"/>
            <link rel = "stylesheet" type = "text/css" href = "style.css"/>
            <title>도서정보 - <xsl:value-of select = "title"/></title>    
        </head>

        <body>

            <img>
                <xsl:attribute name = "src">
                <xsl:value-of select = "image"/>
                </xsl:attribute>
            </img>

            <div id="id3">
                <h2>
                    <xsl:value-of select = "classify"/>&#160;
                    <xsl:value-of select = "title"/>
                </h2>

                <p> 
                    <xsl:value-of select = "author"/>
                </p>

                <p> 
                    <xsl:value-of select = "translater"/>
                </p>

                <p> 
                    <xsl:value-of select = "publish"/>
                </p>

                <p> 
                    <xsl:value-of select = "issued"/>
                </p>

                <p>
                    <xsl:value-of select = "page"/>
                </p>

                <h2>
                    <xsl:value-of select = "price"/>
                </h2>
            </div>
            
        </body>
    </xsl:template>
</xsl:stylesheet>