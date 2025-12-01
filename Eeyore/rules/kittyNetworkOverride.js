/**
 * Mihomo Party 覆写规则 - Kitty Network 专用
 * 基于订阅配置自动生成和优化规则
 */

function main(config) {
  // ========== 基础配置优化 ==========
  
  // 混合端口配置
  config['mixed-port'] = 7890;
  config['allow-lan'] = true;
  config['bind-address'] = '*';
  config['mode'] = 'rule';
  config['log-level'] = 'info';
  config['external-controller'] = '127.0.0.1:9090';
  
  // ========== DNS 配置优化 ==========
  
  config.dns = {
    enable: true,
    listen: '127.0.0.1:1053',
    ipv6: false,
    'default-nameserver': ['223.5.5.5', '119.29.29.29'],
    'enhanced-mode': 'fake-ip',
    'fake-ip-range': '198.18.0.1/16',
    'use-hosts': true,
    nameserver: [
      'https://doh.pub/dns-query',
      'https://dns.alidns.com/dns-query'
    ],
    'fake-ip-filter': [
      'geosite:cn',
      '+.lan',
      'localhost',
      '*.local',
      '*.lan',
      '*.localdomain',
      '*.example',
      '*.invalid',
      '*.localhost',
      '*.test',
      '*.home.arpa',
      // 时间服务器
      'time.*.com',
      'time.*.gov',
      'time.*.edu.cn',
      'time.*.apple.com',
      'time[1-7].*.com',
      'ntp.*.com',
      'ntp[1-7].*.com',
      '*.time.edu.cn',
      '*.ntp.org.cn',
      '+.pool.ntp.org',
      'time1.cloud.tencent.com',
      // 音乐服务
      '*.music.163.com',
      '*.126.net',
      'musicapi.taihe.com',
      'music.taihe.com',
      'songsearch.kugou.com',
      'trackercdn.kugou.com',
      '*.kuwo.cn',
      'api-jooxtt.sanook.com',
      'api.jooxt.com',
      'joox.com',
      'y.qq.com',
      '*.y.qq.com',
      'streamoc.music.tc.qq.com',
      'mobileoc.music.tc.qq.com',
      'isure.stream.qqmusic.qq.com',
      'dl.stream.qqmusic.qq.com',
      'aqqmusic.tc.qq.com',
      'amobile.music.tc.qq.com',
      '*.xiami.com',
      '*.music.migu.cn',
      'music.migu.cn',
      // 微软连接测试
      '+.msftconnecttest.com',
      '+.msftncsi.com',
      // 游戏服务
      '+.srv.nintendo.net',
      '+.stun.playstation.net',
      'xbox.*.microsoft.com',
      '+.xboxlive.com',
      '*.battlenet.com.cn',
      '+.wotgame.cn',
      '+.wggames.cn',
      '+.wowsgame.cn',
      '+.wargaming.net',
      // 其他
      'proxy.golang.org',
      'stun.*.*',
      'stun.*.*.*',
      '+.stun.*.*',
      '+.stun.*.*.*',
      '+.stun.*.*.*.*',
      'heartbeat.belkin.com',
      '*.linksys.com',
      '*.linksyssmartwifi.com',
      '*.router.asus.com',
      'mesu.apple.com',
      'swscan.apple.com',
      'swquery.apple.com',
      'swdownload.apple.com',
      'swcdn.apple.com',
      'swdist.apple.com',
      'lens.l.google.com',
      'stun.l.google.com',
      '+.square-enix.com',
      '*.finalfantasyxiv.com',
      '*.ffxiv.com',
      '*.ff14.sdo.com',
      'ff.dorado.sdo.com',
      'mcdn.bilivideo.cn',
      '+.media.dssott.com',
      '+.pvp.net'
    ]
  };

  // ========== 代理组优化 ==========
  
  // 清理信息节点（这些不是真实的代理节点）
  const infoNodes = [
    '剩余流量：99.51 GB',
    '距离下次重置剩余：8 天',
    '套餐到期：2026-08-05',
    '🇦🇶 Kitty Network',
    '🇦🇶 官网：kitty.su',
    '🇦🇶 TG频道：@kittythenight',
    '🇦🇶 节点仅供个人使用'
  ];

  // 过滤出真实的代理节点
  const realProxies = config.proxies
    .map(p => p.name)
    .filter(name => !infoNodes.includes(name));

  const regionGroups = [
    { label: '🇭🇰 香港节点', keywords: ['香港', 'hong kong', 'hk'] },
    { label: '🇺🇸 美国节点', keywords: ['美国', 'america', 'us', 'united states'] },
    { label: '🇸🇬 狮城节点', keywords: ['新加坡', 'singapore'] },
    { label: '🇯🇵 日本节点', keywords: ['日本', 'japan'] },
    { label: '🇹🇼 台湾节点', keywords: ['台湾', 'taiwan'] },
    { label: '🇰🇷 韩国节点', keywords: ['韩国', 'korea'] },
    { label: '🇪🇺 欧洲节点', keywords: ['英国', 'uk', 'gb', 'england', 'netherlands', '德国', 'deutschland', 'germany', 'france', 'europe'] }
  ];

  const groupedProxies = regionGroups.reduce((acc, group) => {
    acc[group.label] = [];
    return acc;
  }, {});

  const findGroup = (name) => {
    const lowerName = name.toLowerCase();
    return regionGroups.find(group =>
      group.keywords.some(keyword => lowerName.includes(keyword))
    );
  };

  realProxies.forEach(name => {
    const matched = findGroup(name);
    if (matched) {
      groupedProxies[matched.label].push(name);
    }
  });

  const usProxies = groupedProxies['🇺🇸 美国节点'];
  const sgProxies = groupedProxies['🇸🇬 狮城节点'];
  const jpProxies = groupedProxies['🇯🇵 日本节点'];
  const hkProxies = groupedProxies['🇭🇰 香港节点'];
  const twProxies = groupedProxies['🇹🇼 台湾节点'];
  const krProxies = groupedProxies['🇰🇷 韩国节点'];
  const euProxies = groupedProxies['🇪🇺 欧洲节点'];

  // 重新定义代理组
  config['proxy-groups'] = [
    {
      name: '🚀 节点选择',
      type: 'select',
      proxies: ['♻️ 自动选择', '🇭🇰 香港节点', '🇺🇸 美国节点', '🇸🇬 狮城节点', '🇯🇵 日本节点', '🇹🇼 台湾节点', '🇰🇷 韩国节点', '🇪🇺 欧洲节点', 'DIRECT']
    },
    {
      name: '♻️ 自动选择',
      type: 'url-test',
      proxies: realProxies,
      url: 'http://www.gstatic.com/generate_204',
      interval: 300,
      tolerance: 50
    },
    {
      name: '🎯 全球直连',
      type: 'select',
      proxies: ['DIRECT', '🚀 节点选择']
    },
    {
      name: '🛑 广告拦截',
      type: 'select',
      proxies: ['REJECT', 'DIRECT']
    },
    {
      name: '🐟 漏网之鱼',
      type: 'select',
      proxies: ['🚀 节点选择', 'DIRECT']
    }
  ];

  // 添加地区节点组
  if (hkProxies.length > 0) {
    config['proxy-groups'].push({
      name: '🇭🇰 香港节点',
      type: 'url-test',
      proxies: hkProxies,
      url: 'http://www.gstatic.com/generate_204',
      interval: 300,
      tolerance: 50
    });
  }

  if (usProxies.length > 0) {
    config['proxy-groups'].push({
      name: '🇺🇸 美国节点',
      type: 'url-test',
      proxies: usProxies,
      url: 'http://www.gstatic.com/generate_204',
      interval: 300,
      tolerance: 50
    });
  }

  if (sgProxies.length > 0) {
    config['proxy-groups'].push({
      name: '🇸🇬 狮城节点',
      type: 'url-test',
      proxies: sgProxies,
      url: 'http://www.gstatic.com/generate_204',
      interval: 300,
      tolerance: 50
    });
  }

  if (jpProxies.length > 0) {
    config['proxy-groups'].push({
      name: '🇯🇵 日本节点',
      type: 'url-test',
      proxies: jpProxies,
      url: 'http://www.gstatic.com/generate_204',
      interval: 300,
      tolerance: 50
    });
  }

  if (twProxies.length > 0) {
    config['proxy-groups'].push({
      name: '🇹🇼 台湾节点',
      type: 'url-test',
      proxies: twProxies,
      url: 'http://www.gstatic.com/generate_204',
      interval: 300,
      tolerance: 50
    });
  }

  if (krProxies.length > 0) {
    config['proxy-groups'].push({
      name: '🇰🇷 韩国节点',
      type: 'url-test',
      proxies: krProxies,
      url: 'http://www.gstatic.com/generate_204',
      interval: 300,
      tolerance: 50
    });
  }

  if (euProxies.length > 0) {
    config['proxy-groups'].push({
      name: '🇪🇺 欧洲节点',
      type: 'url-test',
      proxies: euProxies,
      url: 'http://www.gstatic.com/generate_204',
      interval: 300,
      tolerance: 50
    });
  }

  // ========== 规则优化 ==========
  
  // 清空原有规则，使用优化后的规则集
  config.rules = [
    // 局域网直连
    'DOMAIN,injections.adguard.org,DIRECT',
    'DOMAIN,local.adguard.org,DIRECT',
    'DOMAIN-SUFFIX,local,DIRECT',
    'IP-CIDR,127.0.0.0/8,DIRECT,no-resolve',
    'IP-CIDR,172.16.0.0/12,DIRECT,no-resolve',
    'IP-CIDR,192.168.0.0/16,DIRECT,no-resolve',
    'IP-CIDR,10.0.0.0/8,DIRECT,no-resolve',
    'IP-CIDR,17.0.0.0/8,DIRECT,no-resolve',
    'IP-CIDR,100.64.0.0/10,DIRECT,no-resolve',
    'IP-CIDR,224.0.0.0/4,DIRECT,no-resolve',
    'IP-CIDR6,fe80::/10,DIRECT,no-resolve',
    'GEOIP,private,DIRECT,no-resolve',
    
    // 广告拦截
    'DOMAIN-KEYWORD,admarvel,🛑 广告拦截',
    'DOMAIN-KEYWORD,admaster,🛑 广告拦截',
    'DOMAIN-KEYWORD,adsage,🛑 广告拦截',
    'DOMAIN-KEYWORD,adsmogo,🛑 广告拦截',
    'DOMAIN-KEYWORD,adsrvmedia,🛑 广告拦截',
    'DOMAIN-KEYWORD,adwords,🛑 广告拦截',
    'DOMAIN-KEYWORD,adservice,🛑 广告拦截',
    'DOMAIN-SUFFIX,appsflyer.com,🛑 广告拦截',
    'DOMAIN-KEYWORD,domob,🛑 广告拦截',
    'DOMAIN-SUFFIX,doubleclick.net,🛑 广告拦截',
    'DOMAIN-KEYWORD,duomeng,🛑 广告拦截',
    'DOMAIN-KEYWORD,dwtrack,🛑 广告拦截',
    'DOMAIN-KEYWORD,guanggao,🛑 广告拦截',
    'DOMAIN-KEYWORD,lianmeng,🛑 广告拦截',
    'DOMAIN-SUFFIX,mmstat.com,🛑 广告拦截',
    'DOMAIN-KEYWORD,mopub,🛑 广告拦截',
    'DOMAIN-KEYWORD,omgmta,🛑 广告拦截',
    'DOMAIN-KEYWORD,openx,🛑 广告拦截',
    'DOMAIN-KEYWORD,partnerad,🛑 广告拦截',
    'DOMAIN-KEYWORD,pingfore,🛑 广告拦截',
    'DOMAIN-KEYWORD,supersonicads,🛑 广告拦截',
    'DOMAIN-KEYWORD,uedas,🛑 广告拦截',
    'DOMAIN-KEYWORD,umeng,🛑 广告拦截',
    'DOMAIN-KEYWORD,usage,🛑 广告拦截',
    'DOMAIN-SUFFIX,vungle.com,🛑 广告拦截',
    'DOMAIN-KEYWORD,wlmonitor,🛑 广告拦截',
    'DOMAIN-KEYWORD,zjtoolbar,🛑 广告拦截',
    
    // Telegram
    'DOMAIN-SUFFIX,telegra.ph,🚀 节点选择',
    'DOMAIN-SUFFIX,telegram.org,🚀 节点选择',
    'IP-CIDR,91.108.4.0/22,🚀 节点选择,no-resolve',
    'IP-CIDR,91.108.8.0/21,🚀 节点选择,no-resolve',
    'IP-CIDR,91.108.16.0/22,🚀 节点选择,no-resolve',
    'IP-CIDR,91.108.56.0/22,🚀 节点选择,no-resolve',
    'IP-CIDR,149.154.160.0/20,🚀 节点选择,no-resolve',
    'IP-CIDR6,2001:67c:4e8::/48,🚀 节点选择,no-resolve',
    'IP-CIDR6,2001:b28:f23d::/48,🚀 节点选择,no-resolve',
    'IP-CIDR6,2001:b28:f23f::/48,🚀 节点选择,no-resolve',
    
    // Google 服务
    'DOMAIN-KEYWORD,google,🚀 节点选择',
    'DOMAIN-KEYWORD,gmail,🚀 节点选择',
    'DOMAIN-KEYWORD,youtube,🚀 节点选择',
    'DOMAIN-SUFFIX,youtu.be,🚀 节点选择',
    'DOMAIN-SUFFIX,gstatic.com,🚀 节点选择',
    'DOMAIN-SUFFIX,ggpht.com,🚀 节点选择',
    'DOMAIN-SUFFIX,ytimg.com,🚀 节点选择',
    'DOMAIN-SUFFIX,googleapis.cn,🚀 节点选择',
    
    // 社交媒体
    'DOMAIN-KEYWORD,facebook,🚀 节点选择',
    'DOMAIN-SUFFIX,fb.me,🚀 节点选择',
    'DOMAIN-SUFFIX,fbcdn.net,🚀 节点选择',
    'DOMAIN-KEYWORD,twitter,🚀 节点选择',
    'DOMAIN-SUFFIX,twimg.com,🚀 节点选择',
    'DOMAIN-KEYWORD,instagram,🚀 节点选择',
    'DOMAIN-KEYWORD,whatsapp,🚀 节点选择',
    
    // GitHub
    'DOMAIN-KEYWORD,github,🚀 节点选择',
    'DOMAIN-SUFFIX,git.io,🚀 节点选择',
    
    // 国内直连
    'GEOSITE,cn,🎯 全球直连',
    'GEOIP,CN,🎯 全球直连',
    
    // 最终规则
    'MATCH,🐟 漏网之鱼'
  ];

  return config;
}
