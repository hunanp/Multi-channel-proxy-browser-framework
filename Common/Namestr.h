#ifndef __HTTP_Localstr_H__  
#define __HTTP_Localstr_H__ 
#include <windows.h>
#include <string.h>
#include <unordered_map>
#include <atlstr.h>
using namespace std;
namespace ImWindow
{
	class StringPair
	{
	public:
		StringPair(void) {};
		~StringPair(void) {};
		static void GetChineseCity(std::unordered_map<std::string, std::string> &code)
		{

		}
	
		static void CountryCode(std::unordered_map<std::string, std::string> &code)
		{

		}
		static void LoadLocalString(int codeindex,std::unordered_map<std::string, char*> &localstring)
		{//各语言版本
			switch (codeindex)
			{
			default:
				localstring.clear();
				//工具条：
				localstring["Back"] = u8"后退";
				localstring["Forward"] = u8"前进";
				localstring["Stop"] = u8"停止";
				localstring["Refresh"] = u8"刷新";
				localstring["Max"] = u8"最大化";
				localstring["WebUrl"] = u8"网址";
				localstring["MultiScreen"] = u8"多屏/单屏显示";
				localstring["Proxy"] = u8"代理";
				localstring["TestAll"] = u8"检测全部";
				localstring["StopTestAll"] = u8"停止测试";
				localstring["TestAllHelp"] = u8"测试或停止测试下载速度";
				localstring["Setting"] = u8"设置";
				localstring["Newtab"] = u8"打开新标签";
				localstring["Close"] = u8"关闭";
				localstring["Searching"] = "正在搜索，是否真要退出？";
				localstring["info"] = "提示";
				//Forms.Setting
				localstring["Tabs"] = u8"标签样式";
				localstring["Chrome"] = u8"双斜坡";
				localstring["Square"] = u8"直角";
				localstring["Default"] = u8"直角斜坡";
				localstring["Showborder"] = u8"显示边框";
				localstring["MultiScreenSet"] = u8"多屏设置";
				localstring["Row"] = u8"行";
				localstring["Column"] = u8"列";
				localstring["Zoom"] = u8"网页缩放";
				localstring["MultiScreen"] = u8"多屏";
				localstring["SingleScreen"] = u8"单屏";

				localstring["Languge"] = u8"语言";
				localstring["Korean"] = u8"한국어";
				localstring["Japanese"] = u8"こんにちは！テスト";
				localstring["French"] = u8"Français";
				localstring["Spanish"] = u8"Español";
				localstring["Portuguese"] = u8"Português";
				localstring["Arabic"] = u8"اللغة العربية";
				localstring["Russian"] = u8"Русский";
				localstring["Simplified"] = u8"中文(简体)";
				localstring["Traditional"] = u8"中文(繁體)";

				localstring["FontSize"] = u8"字体";
				localstring["Type"] = u8"类型";
				localstring["Host"] = u8"服务器";
				localstring["UserName"] = u8"用户名";
				localstring["Password"] = u8"密码";
				localstring["Encryption"] = u8"密码/加密";
				localstring["Protocol"] = u8"协议";
				localstring["Port"] = u8"端口";
				localstring["Add"] = u8"新增";
				localstring["Addhelp"] = u8"新增记录";
				localstring["Import"] = u8"导入";
				localstring["ImportList"] = u8"导入数据";
				localstring["Export"] = u8"导出";
				localstring["ExportList"] = u8"导出表格内容";
				localstring["Addhelp"] = u8"添加代理.可使用 TAB/SHIFT+TAB 循环移动.";
				localstring["importhelp"] = u8"导入格式按类型，主机,用户名,密码,端口用逗号分隔，逐行排列。";
				localstring["Exporthelp"] = u8"导出代理列表";
				localstring["List"] = u8"列表";
				localstring["Delete"] = u8"删除";
				localstring["DefaultURL"] = u8"默认网址";
				localstring["Save"] = u8"保存";
				localstring["SaveAll"] = u8"保存全部";
				localstring["Cancel"] = u8"取消";
				localstring["Name"] = u8"名称";
				//Forms.CrawlerWindow
				localstring["Crawler"] = u8"网络蜘蛛";
				localstring["UrlKeyword"] = u8" 网址或关键字:";
				localstring["UrlKeywordTip"] = u8"输入网址或关键字回车或点搜索图标启动搜索";
				localstring["SearchEngine"] = u8"搜索引擎";
				localstring["SearchEngineTip"] = u8"选择需要的搜索引擎";
				localstring["Threads"] = u8"线程数";
				localstring["ThreadsTip"] = u8"调整搜索线程数。可按鼠标左键拖动调整。";
				localstring["Search"] = u8"开始搜索";
				localstring["Thread"] = u8"线程";
				localstring["Horizontal"] = u8"横向";
				localstring["Vertical"] = u8"纵向";
				localstring["Progress"] = u8"进度";
				localstring["ThreadList"] = u8"  搜索线程  ";
				localstring["EmailList"] = u8"  邮箱列表  ";
				localstring["NumberList"] = u8"  号码列表  ";
				localstring["ErrorList"] = u8"  错误信息  ";
				localstring["GeneralSettings"] = u8"  通用设置  ";
				localstring["NumberFormat"] = u8"  号码格式设置  ";
				localstring["Email"] = u8"邮箱";
				localstring["Number"] = u8"号码";
				localstring["Title"] = u8"标题";
				localstring["Error"] = u8"错误";
				localstring["Finished"] = u8"完成";
				localstring["Number"] = u8"号码";
				localstring["Total"] = u8"共";
				localstring["Pages"] = u8"页";
				localstring["RunTime"] = u8"运行时间";
				localstring["Play"] = u8"开始或继续搜索";
				localstring["Pause"] = u8"暂停";
				localstring["Stop"] = u8"停止";
				localstring["GetKey"] = u8"获得注册码";
				localstring["Contact"] = u8"联系我们";
				localstring["DeleteLine"] = u8"删除该行";
				localstring["ClearEmailList"] = u8"清空邮箱列表";
				localstring["ClearNumberList"] = u8"清空号码列表";
				localstring["ClearErrorList"] = u8"清空错误信息";
				localstring["AddToFavoriteList"] = u8"添加到收藏夹";
				localstring["Favorite"] = u8"收藏夹";
				localstring["ExtractEmail"] = u8"邮箱";
				localstring["ExtractNumber"] = u8"手机号码";
				localstring["ExtractItem"] = u8"搜索项目";
				localstring["LinkTimeOut"] = u8"连接超时";
				localstring["DownLoadTimeOut"] = u8"下载超时";
				localstring["Second"] = u8"(秒)";
				localstring["Minute"] = u8"(分钟)";
				localstring["Option"] = u8"选项";
				localstring["AdvancedSetting"] = u8"高级设置";
				localstring["TotalSearch"] = u8"总搜索量";
				localstring["OneSite"] = u8"单网站量";
				localstring["TimeInterval"] = u8"备份间隔";
				localstring["BakPath"] = u8"备份路径";
				localstring["LinePerPage"] = u8"每页行数";
				localstring["SlectPath"] = "选文件夹目录";//传入WIN不能用u8多字节
				localstring["Include"] = u8"包含：(即网页或网址包括这些内容才会提取)";
				localstring["Exclude"] = u8"排除：(即网页或网址包括这些内容则不提取)";
				localstring["AdditionalKeywords"] = u8"附加关键字";
				localstring["Content"] = u8"网页内容";
				localstring["EmailOrNum"] = u8"邮箱或号码";
				localstring["Keywords"] = u8"关键字";
				localstring["Default"] = u8"默认值";
				localstring["Country"] = u8"国家";
				localstring["CountryCode"] = u8"代码";
			
				localstring["SearchValue"] = u8"查找";
				localstring["First"] = u8"首页";
				localstring["Previous"] = u8"上一页";
				localstring["Jump"] = u8"快速跳转到指定页，可拖动";
				localstring["Next"] = u8"下一页";
				localstring["Last"] = u8"最后";
				localstring["SearchValueTip"] = u8"输入查找值然后回车，可过滤出需要的数据导出。";
				localstring["Help"] = u8"帮助";
				localstring["About"] = u8"关于";
				localstring["SoftName"] = u8"网络爬虫机器人";
				localstring["Company"] = u8"Kirosoft";
				localstring["Copyright"] = u8"Copyright";
				localstring["Companyurl"] = u8"www.tradesir.com";
				localstring["MachineCode"] = u8"机器码";
				localstring["KeyCode"] = u8"注册码";
				localstring["Register"] = u8"注册";
				localstring["Cancel"] = u8"取消";
				localstring["Title"] = u8"标题";
				localstring["Content"] = u8"内容";
				localstring["Send"] = u8"发送";
				localstring["YourEmail"] = u8"您的邮箱";
				localstring["RegSuccessful"] = u8"注册成功!";
				localstring["RegFailed"] = u8"注册失败!";
				localstring["Message"] = u8"消息";
				localstring["Location"] = u8"归属地";
				localstring["ExtractUrl"] = u8"提取IP网址";
				localstring["ExtractUrlInfo"] = u8"输入需要提取IP的网址，回车";
				localstring["ExtractCountry"] = u8"国家或地区";
				localstring["ExtractCountryInfo"] = u8"只提取指定国家或地区的代理IP";
				localstring["NoLimit"] = u8"不限";
				localstring["ExcludeCN"] = u8"排除中国大陆IP";
				localstring["ConectTime"] = u8"连接时间";
				localstring["TypeInfo"] = u8"只提取指定类型的代理IP";
				localstring["GetSubItem"] = u8"更新订阅";
				localstring["Update"] = u8"更新";
				localstring["EditSubItem"] = u8"编辑订阅";
				localstring["AutoUpdateSub"] = u8"自动更新订阅";
				localstring["GetSubItemhelp"] = u8"从订阅链接获得新的代理帐号";
				localstring["EditSubItemhelp"] = u8"维护代理订阅链接";
				localstring["SubItemURL"] = u8"订阅网址";
				localstring["InputFileType"] = u8"可输入代理信息串或保存代理信息的文件名如：file://D:/Work/vpnfile/vmess.txt";
				localstring["Clear"] = u8"清空";
				localstring["ClearList"] = u8"清空表格内容";
				localstring["Speed"] = u8"速度(b/s)";
				localstring["Response"] = u8"响应";
				localstring["NoResponse"] = u8"未响应";
				localstring["Select"] = u8"选择";
				localstring["Active"] = u8"启用";
				localstring["ActiveHelp"] = u8"选择即启用此代理，同时选多个则自动切换。";
				localstring["NoProxy"] = u8"不使用代理";
				localstring["NoProxyHelp"] = u8"取消所有代理";
				localstring["ProxyAllHelp"] = u8"启用全部或取消所有选择";
				localstring["GlobalProxy"] = u8"全局代理";
				localstring["GlobalProxyHelp"] = u8"包括IE、Edge、Chrome等浏览器均使用此代理";
				localstring["LocalProxy"] = u8"局部代理";
				localstring["LocalProxyHelp"] = u8"仅在此软件中使用代理";
				localstring["PACProxy"] = u8"智能分流";
				localstring["PACProxyHelp"] = u8"自动按PAC表决定哪些网站用代理访问";
				localstring["AutoChangeProxy"] = u8"自动换IP";
				localstring["AutoChangeProxyHelp"] = u8"代理失效时自动切换到另一个启用的有效代理";
				localstring["EditPACList"] = u8"编辑PAC列表";
				localstring["EditPACListHelp"] = u8"维护PAC资料";
				localstring["ProxyDomain"] = u8"代理域名";
				localstring["DirectDomain"] = u8"直连域名";
				localstring["General"] = u8"常规";
				localstring["SelectAll"] = u8"全选择";
				localstring["CancelAll"] = u8"全取消";
				localstring["Serverunavailable"] = u8"(服务器无响应)";
				localstring["DeleUnusable"] = u8"删无响应";
				localstring["DeleUnusableHelp"] = u8"删除无响应代理";
				localstring["ActiveAll"] = u8"全部启用";
				localstring["DisactiveAll"] = u8"全部取消";
				localstring["SortHelp"] = u8"按升序或降序排列";
				localstring["TestDownloadHelp"] = u8"测试下载速度";
				localstring["WaitFinished"] = u8"数据处理中，完成后才能删除。";
				localstring["ThreadRunning"] = u8"作业线程正在运行，需要完成后才能进行此项操作。";
				localstring["WorkFinished"] = u8"操作完成。";
				localstring["VPNString"] = u8"URL";
				localstring["Yes"] = u8"是";
				localstring["Sort"] = u8"点击标题栏按 ## 的升序或降序排列。";
				localstring["CanGlobal"] = u8"是否可用于全局代理设置和自动匹配代理设置";
				localstring["Global"] = u8"全局";
				localstring["TesthttpSpeed"] = u8"此列为http下载速度，不一定支持https.";
				localstring["CopyToClipboard"] = u8"已经复制到粘贴板。";
				localstring["ProxyTestURL"] = u8"指定测试网址";
				localstring["ProxyTestURLTIP"] = u8"注意，若指定网址已被封锁可能导至所有代理验证失败。";
				localstring["Browse"] = u8"浏览";
				localstring["SelectTextHelp"] = u8"打开网页，点击需要提取的文本可以指定提取内容。";
				localstring["TestTime"] = u8"测试时间：";
				localstring["Browser"] = u8"浏览器：";
				localstring["Builtin"] = u8"内置浏览器：";
				localstring["NoNeed"] = u8"不需要：";
				localstring["WindowOpened"] = u8"窗口已经打开。";
				//这四个只用在TIP提示
				localstring["Position"] = "坐标:";
				localstring["ID"] = "    ID:";
				localstring["Text"] = "文本:";
				localstring["Link"] = "链接:";
				localstring["Test"] = u8"测试";
				localstring["Direct"] = u8"直接提取";
				localstring["Builtinsimulation"] = u8"内置浏览器";
				localstring["UseEdge"] = u8"使用Edge";
				localstring["UseChrome"] = u8"使用Chrome";
				localstring["UseProxy"] = u8"使用代理";
				localstring["UseProxyHelp"] = u8"是否使用代理访问";
				localstring["ExtractionMethod"] = u8"提取方式";
				localstring["Coordinate"] = u8"指定坐标";
				localstring["ElementID"] = u8"指定元素ID";
				localstring["Result"] = u8"提取结果";
				localstring["IDhelp"] = u8"打开订阅网址。若在网页上点击鼠标，系统会记录点击位置的元素ID和坐标，用浏览器提取时只取该位置的内容。";
				localstring["DirectHelp"] = u8"直连网站下载";
				localstring["BuiltinHelp"] = u8"使用内置浏览器模拟打开";
				localstring["NeedInstallEdge"] = u8"用Edge下载。需要在系统中安装好Edge";
				localstring["NeedInstallChrome"] = u8"用Chrome下载。需要在系统中安装好Chrome";
				localstring["noText"] = u8"未能提取到内容。\n 1 确认网址是否可打开。\n 2 可修改提取方式。\n 3 设了ID或坐标的可换个位置试试。\n 4 用了代理的确认代理是否可用。";
				localstring["OpenError"] = u8"打开网页出错\n 1 请确认此时网址是否可以访问\n 2 用了代理的请检查代理是否可用\n 3 访问方式可能不被支持";
				localstring["SubItemSet"] = u8"订阅设置";
				localstring["loadfail"] = u8"提取失败";
				localstring["SelectID"] = u8"浏览及选取ID和坐标";
				localstring["loading"] = u8"正在加载…";
				localstring["usingsite"] = u8"应用到网站";
				localstring["usingsiteHelp"] = u8"该网站的所有网址按此方式提取";
				localstring["ChangeIPTime"] = u8"换IP间隔";
				localstring["SearchValueHelp"] = u8"输入查找内容即可过滤出相应数据";

				localstring["SpeedTest"] = u8"速度测试";
				localstring["WebTest"] = u8"网页测试";
				localstring["WebTestHelp"] = u8"测试用此代理打开网页";
				//菜单
				localstring["MenuNewTabOpen"] = "新标签页中打开|(Ctrl+&O)";
				localstring["MenuHttpSpeedTest"] = "速度测试(http)|(Ctrl+&A)";
				localstring["MenuHttpsSpeedTest"] = "速度测试(https)|(Ctrl+&A)";
				localstring["MenuAllHttpSpeed"] = "全部速度(http)|(Ctrl+&W)";
				localstring["MenuAllHttpsSpeed"] = "全部速度(https)|(Ctrl+&M)";
				localstring["MenuCopyProxy"] = "分享链接|(Ctrl+&C)";
				localstring["MenuImportProxy"] = "导入链接|(Ctrl+&V)";
				localstring["MenuDeleteRow"] = "删除该行|(Ctrl+&D)";
				localstring["MenuClearList"] = "清空列表|(Ctrl+&L)";
				localstring["MenuDeleteNoResponse"] = "删除无响应代理|(Ctrl+&S)";
				localstring["MenuDeleteZeroSpeed"] = "删除零速度代理|(Ctrl+&K)";
				localstring["MenuInportList"] = "导入列表|(Ctrl+&I)";
				localstring["MenuExportList"] = "导出列表|(Ctrl+&E)";
				localstring["MenuRenewSubItem"] = "更新订阅|(Ctrl+&N)";
				localstring["MenuEditSubItem"] = "编辑订阅|(Ctrl+&B)";
				localstring["MenuEditPAC"] = "编辑分流表|(Ctrl+&N)";
				localstring["MenuImageSaveAs"] = "将图片另存为|(Ctrl+&M)";
				localstring["MenuForward"] = "前进|(Ctrl+&F)";
				localstring["MenuBack"] = "后退|(Ctrl+&B)"; 
				localstring["MenuRefresh"] = "刷新|(Ctrl+&R)";
				localstring["MenuStop"] = "停止|(Ctrl+&T)";
				localstring["MenuDelete"] = "删除|(Ctrl+&D)";
				localstring["MenuNewWindowOpen"] = "新窗口中打开|(Ctrl+&N)";
				localstring["MenuSaveAs"] = "将目标另存为|(Ctrl+&S)";
				localstring["MenuCpoyLink"] = "复制链接|(Ctrl+&C)";
				localstring["MenuCpoy"] = "复制|(Ctrl+&C)";
				localstring["MenuPaste"] = "粘贴|(Ctrl+&V)";
				localstring["MenuCut"] = "剪切|(Ctrl+&X)";
				localstring["MenuUndo"] = "撤销|(Ctrl+&Z)";
				localstring["MenuSelectAll"] = "全选|(Ctrl+&A)";
				localstring["MenuRedo"] = "重做|(Ctrl+&R)";
				localstring["MenuEdit"] = "修改|(Ctrl+&R)";
				localstring["MenuClear"] = "清空|(Ctrl+&L)";
				localstring["MenuOpen"] = "打开|(Ctrl+&O)";
				localstring["MenuInport"] = "导入|(Ctrl+&O)";
				localstring["MenuExport"] = "导出|(Ctrl+&S)";
				break;
			}
		}
	};
}
#endif  