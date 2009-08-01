<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbsleft">
		<html>
			<head>
				<title>主菜单</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbsleft.css" />
				<script type="text/javascript" src="/js/bbsleft.js"></script>
			</head>
			<body>
				<div class="main">
					<a href="#" onclick="switch_bar()" id="switchbar">&#160;</a>
					<div id="mainbar">
						<!-- 本站精华 -->
						<a href="0an" target="view"><img src="/images/announce.gif" />本站精华</a>
						<!-- 全部讨论 -->
						<a href="all" target="view"><img src="/images/penguin.gif" />全部讨论</a>
						<!-- 统计数据 -->
						<a href="#" onclick="return SwitchPanel('Stat')"><img src="/images/top10.gif" />统计数据</a>
						<div id="Stat">
							<a href="top10" target="view"><img src="/images/blankblock.gif" />本日十大</a>
							<a href="topb10" target="view"><img src="/images/blankblock.gif" />热门讨论</a>
							<a href="userinfo" target="view"><img src="/images/blankblock.gif" />在线统计</a>
						</div>
						<!-- 我的收藏 -->
						<xsl:if test="favbrd">
							<a href="#" onclick="return SwitchPanel('Favorite')"><img src="/images/favorite.gif" />我的收藏</a>
							<div id="Favorite">
								<a href="mybrd" target="view"><img src="/images/blankblock.gif" />预定管理</a>
								<xsl:apply-templates select="favbrd" />
							</div>
						</xsl:if>
						<!-- 鹊桥相会 -->
						<a href="#" onclick="return SwitchPanel('QueQiao')"><img src="/images/chat.gif" />鹊桥相会</a>
						<div id="QueQiao">
							<xsl:if test="login='1'">
								<a href="friend" target="view"><img src="/images/blankblock.gif" />在线好友</a>
							</xsl:if>
							<a href="friend" target="view"><img src="/images/blankblock.gif" />环顾四方</a>
							<xsl:if test="talk!='0'">
								<a href="sendmsg" target="view"><img src="/images/blankblock.gif" />发送讯息</a>
								<a href="msg" target="view"><img src="/images/blankblock.gif" />查看所有讯息</a>
							</xsl:if>
						</div>
						<xsl:if test="login='1'">
							<!-- 个人设置 -->
							<a href="#" onclick="return SwitchPanel('Config')"><img src="/images/config.gif" />个人设置</a>
							<div id="Config">
								<a href="info" target="view"><img src="/images/blankblock.gif" />个人资料</a>
								<a href="plan" target="view"><img src="/images/blankblock.gif" />改说明档</a>
								<a href="sig" target="view"><img src="/images/blankblock.gif" />改签名档</a>
								<a href="mywww" target="view"><img src="/images/blankblock.gif" />WWW定制</a>
								<a href="pwd" target="view"><img src="/images/blankblock.gif" />修改密码</a>
								<a href="nick" target="view"><img src="/images/blankblock.gif" />临时改昵称</a>
								<a href="fall" target="view"><img src="/images/blankblock.gif" />设定好友</a>
								<xsl:if test="cloak!='0'">
									<a href="cloak" target="view"><img src="/images/blankblock.gif" />切换隐身</a>
								</xsl:if>
							</div>
							<!-- 处理信件 -->
							<a href="#" onclick="return SwitchPanel('Mail')"><img src="/images/mail.gif" />处理信件</a>
							<div id="Mail">
								<a href="newmail" target="view"><img src="/images/blankblock.gif" />阅览新信件</a>
								<a href="mail" target="view"><img src="/images/blankblock.gif" />所有信件</a>
								<a href="maildown" target="view"><img src="/images/blankblock.gif" />下载信件</a>
								<a href="pstmail" target="view"><img src="/images/blankblock.gif" />发送信件</a>
							</div>
						</xsl:if>
						<!-- 查找选项 -->
						<a href="#" onclick="return SwitchPanel('Search')"><img src="/images/search.gif" />查找选项</a>
						<div id="Search">
							<xsl:if test="find!='0'">
								<a href="find" target="view"><img src="/images/blankblock.gif" />查找文章</a>
							</xsl:if>
							<xsl:if test="login='1'">
								<a href="qry" target="view"><img src="/images/blankblock.gif" />查询网友</a>
							</xsl:if>
							<a href="sel" target="view"><img src="/images/blankblock.gif" />查找讨论区</a>
						</div>
						<!-- 终端登录 -->
						<a href="telnet://bbs.fudan.sh.cn:23"><img src="/images/telnet.gif" />终端登录</a>
						<!-- 注销登录 -->
						<xsl:if test="login='1'">
								<a href="logout" target="_top"><img src="/images/exit.gif" />注销登录</a>
						</xsl:if>
					</div>
				</div>
			</body>
		</html>
	</xsl:template>

	<xsl:template match="favbrd">
		<xsl:for-each select="./*">
			<xsl:if test="name()='dir'">
				<a target="view">
					<xsl:attribute name="href">boa?board=<xsl:value-of select="." /></xsl:attribute><xsl:value-of select="." />
				</a>
			</xsl:if>
			<xsl:if test="name()='board'">
				<a target="view">
					<xsl:attribute name="href"><xsl:value-of select="/bbsleft/favurl" />?board=<xsl:value-of select="." /></xsl:attribute><xsl:value-of select="." />
				</a>
			</xsl:if>
		</xsl:for-each>
	</xsl:template>
</xsl:stylesheet>
