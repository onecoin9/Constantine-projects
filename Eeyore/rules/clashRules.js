/**
 * Clash-Verge-Rev 全局扩展脚本
 *
 * @Version 1.3
 *
 * @description
 * 通过模块化配置，实现对订阅链接自定义重写
 *
 * @customization
 * 1. 自定义规则和过滤节点
 * 2. 自定义链接代理落地节点
 * 3. 开启/关闭/添加服务
 * 4. 管理节点区域
 */

// ===================================================================================
// 1. 自定义规则和过滤节点
// ===================================================================================
const EXCLUDED_KEYWORDS = [
    '官网', '到期', '流量', '剩余', '时间', '重置', '订阅', '卡顿',
    'Kitty Network', 'kitty.su', 'TG频道', '仅供个人使用'
];

// 如果您在此处硬编码了策略组或地区分组（如'US-手动选择'），请务必确保您的订阅中始终包含对应策略或地区。
const CustomizationRule = [
    //"DOMAIN-SUFFIX,jetbrains.ai,US-手动选择",
    //"DOMAIN-SUFFIX,jetbrains.ai,ChatGPT",
    "DOMAIN-SUFFIX,jetbrains.ai,节点选择",
    "PROCESS-NAME,tailscaled,DIRECT",
    "PROCESS-NAME,tailscaled.exe,DIRECT",
    "DOMAIN-SUFFIX,mcdn.bilivideo.com,REJECT",
    "DOMAIN-SUFFIX,mcdn.bilivideo.cn,REJECT",
    "DOMAIN-SUFFIX,szbdyd.com,REJECT",
];

// ===================================================================================
// 2. 自定义链接代理落地节点
// ===================================================================================
const chainTransitName = "链式中转";
const chainLandingProxies = [
    {
        "name": "猫猫-US-ATT",
        "type": "socks5",
        "server": "1.1.1.1",
        "port": 46688,
        "username": "username",
        "password": "password",
        "dialer-proxy": chainTransitName
    }
];

// ===================================================================================
// 3. 服务模块化配置区 (在此开启/关闭或添加服务)
//  'openai': {
//         enabled: true,              // 是否添加此规则组
//         allowDirect: false,         // 为规则组添加直连
//         groupName: 'ChatGPT',       // 规则组显示的名称
//         icon: '...',                // 规则组显示的图标
//         regions: ['US', 'SG', 'JP'],  // 规则组过滤地区节点, [] 表示所有, 具体区域查看 4. 区域枚举配置区
//         rule: {
//              providerKey: 'openai',   // 规则标识
//              url: '...'             // 规则地址, 来自项目: blackmatrix7/ios_rule_script, MetaCubeX/meta-rules-dat, Loyalsoldier/clash-rules
//         }
//  },
// ===================================================================================
const ENABLED_SERVICES = {
    // AI 服务
    'openai': {
        enabled: true,
        allowDirect: false,
        groupName: 'ChatGPT',
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/chatgpt.svg',
        regions: ['US'],
        rule: {
            providerKey: 'openai',
            url: 'https://fastly.jsdelivr.net/gh/blackmatrix7/ios_rule_script@master/rule/Clash/OpenAI/OpenAI.yaml'
        }
    },
    'anthropic': {
        enabled: true,
        allowDirect: false,
        groupName: 'Claude',
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/claude.svg',
        regions: ['US'],
        rule: {
            providerKey: 'anthropic',
            url: 'https://raw.githubusercontent.com/MetaCubeX/meta-rules-dat/refs/heads/meta/geo/geosite/classical/anthropic.yaml'
        }
    },
    // 常用服务
    'telegram': {
        enabled: true,
        allowDirect: false,
        groupName: 'Telegram',
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/telegram.svg',
        regions: ['SG'],
        rule: {
            providerKey: 'telegram',
            url: 'https://raw.githubusercontent.com/blackmatrix7/ios_rule_script/master/rule/Clash/Telegram/Telegram.yaml',
            options: 'no-resolve'
        }
    },
    'github': {
        enabled: true,
        allowDirect: false,
        groupName: 'Github',
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/github.svg',
        regions: ['HK', 'SG', 'JP'],
        rule: {
            providerKey: 'github',
            url: 'https://raw.githubusercontent.com/MetaCubeX/meta-rules-dat/refs/heads/meta/geo/geosite/classical/github.yaml'
        }
    },
    'google': {
        enabled: true,
        allowDirect: false,
        groupName: 'Google',
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/google.svg',
        regions: ['US'],
        rule: {
            providerKey: 'google',
            url: 'https://raw.githubusercontent.com/blackmatrix7/ios_rule_script/master/rule/Clash/Google/Google.yaml'
        }
    },
    'microsoft': {
        enabled: true,
        allowDirect: true,
        groupName: 'MicroSoft',
        icon: 'https://www.clashverge.dev/assets/icons/microsoft.svg',
        regions: [],
        rule: {
            providerKey: 'microsoft',
            url: 'https://raw.githubusercontent.com/blackmatrix7/ios_rule_script/master/rule/Clash/Microsoft/Microsoft.yaml'
        }
    },
    'apple': {
        enabled: true,
        allowDirect: true,
        groupName: 'Apple',
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/apple.svg',
        regions: [],
        rule: {
            providerKey: 'apple',
            url: 'https://raw.githubusercontent.com/blackmatrix7/ios_rule_script/master/rule/Clash/Apple/Apple_Classical.yaml'
        }
    },
    // 流媒体服务
    'netflix': {
        enabled: true,
        allowDirect: false,
        groupName: 'Netflix',
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/netflix.svg',
        regions: ['HK', 'SG', 'JP', 'US'],
        rule: {
            providerKey: 'netflix',
            url: 'https://raw.githubusercontent.com/blackmatrix7/ios_rule_script/master/rule/Clash/Netflix/Netflix_Classical.yaml'
        }
    },
};

// ===================================================================================
// 4. 区域枚举配置区 (管理节点地区)
// ===================================================================================
const REGIONS = {
    HK: {
        name: '香港',
        regex: /香港|HK|Hong Kong|🇭🇰/,
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/flags/hk.svg'
    },
    TW: {
        name: '台湾',
        regex: /台湾|TW|Taiwan|🇨🇳|🇹🇼/,
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/flags/tw.svg'
    },
    SG: {
        name: '新加坡',
        regex: /新加坡|狮城|SG|Singapore|🇸🇬/,
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/flags/sg.svg'
    },
    JP: {
        name: '日本',
        regex: /日本|JP|Japan|🇯🇵/,
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/flags/jp.svg'
    },
    US: {
        name: '美国',
        regex: /美国|US|United States|America|🇺🇸/,
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/flags/us.svg'
    },
    KR: {
        name: '韩国',
        regex: /韩国|KR|Korea|🇰🇷/,
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/flags/kr.svg'
    },
    TR: {
        name: '土耳其',
        regex: /土耳其|TR|Turkey|Türkiye|🇹🇷/,
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/flags/tr.svg'
    },
    NL: {
        name: '荷兰',
        regex: /荷兰|NL|Netherlands|🇳🇱/,
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/flags/nl.svg'
    },
    GB: {
        name: '英国',
        regex: /英国|UK|GB|United Kingdom|Britain|🇬🇧/,
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/flags/gb.svg'
    },
    DE: {
        name: '德国',
        regex: /德国|DE|Germany|🇩🇪/,
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/flags/de.svg'
    },
};

// ===================================================================================
// 5. 底层配置 (基本无需修改)
// ===================================================================================
const chainLandingName = "链式落地";
const groupBaseOption = {
    "interval": 0,
    "timeout": 3000,
    "url": "https://www.google.com/generate_204",
    "lazy": true,
    "max-failed-times": 3,
    "hidden": false
};
const ruleProviderCommon = {
    "type": "http",
    "format": "yaml",
    "interval": 86400
};
const staticRuleProviders = {
    "reject": { ...ruleProviderCommon, "behavior": "domain", "url": "https://fastly.jsdelivr.net/gh/Loyalsoldier/clash-rules@release/reject.txt", "path": "./ruleset/loyalsoldier/reject.yaml" },
    "proxy": { ...ruleProviderCommon, "behavior": "domain", "url": "https://fastly.jsdelivr.net/gh/Loyalsoldier/clash-rules@release/proxy.txt", "path": "./ruleset/loyalsoldier/proxy.yaml" },
    "direct": { ...ruleProviderCommon, "behavior": "domain", "url": "https://fastly.jsdelivr.net/gh/Loyalsoldier/clash-rules@release/direct.txt", "path": "./ruleset/loyalsoldier/direct.yaml" },
    "cncidr": { ...ruleProviderCommon, "behavior": "ipcidr", "url": "https://fastly.jsdelivr.net/gh/Loyalsoldier/clash-rules@release/cncidr.txt", "path": "./ruleset/loyalsoldier/cncidr.yaml" },
    "lancidr": { ...ruleProviderCommon, "behavior": "ipcidr", "url": "https://fastly.jsdelivr.net/gh/Loyalsoldier/clash-rules@release/lancidr.txt", "path": "./ruleset/loyalsoldier/lancidr.yaml" },
    "applications": { ...ruleProviderCommon, "behavior": "classical", "url": "https://fastly.jsdelivr.net/gh/Loyalsoldier/clash-rules@release/applications.txt", "path": "./ruleset/loyalsoldier/applications.yaml" },
    "private": { ...ruleProviderCommon, "behavior": "domain", "url": "https://raw.githubusercontent.com/MetaCubeX/meta-rules-dat/refs/heads/meta/geo/geosite/classical/private.yaml", "path": "./ruleset/MetaCubeX/private.yaml" },
    "gfw": { ...ruleProviderCommon, "behavior": "domain", "url": "https://raw.githubusercontent.com/MetaCubeX/meta-rules-dat/refs/heads/meta/geo/geosite/classical/gfw.yaml", "path": "./ruleset/MetaCubeX/gfw.yaml" },
};
const staticRules = {
    top: [
        ...CustomizationRule,
        "RULE-SET,applications,DIRECT",
        "RULE-SET,private,DIRECT",
        "RULE-SET,reject,广告过滤"
    ],
    bottom: [
        "RULE-SET,proxy,节点选择",
        "RULE-SET,gfw,节点选择",
        "RULE-SET,direct,DIRECT,no-resolve",
        "RULE-SET,lancidr,DIRECT,no-resolve",
        "RULE-SET,cncidr,DIRECT,no-resolve",
        "GEOIP,LAN,DIRECT,no-resolve",
        "GEOIP,CN,DIRECT,no-resolve",
        "MATCH,漏网之鱼"
    ]
};
const domesticNameservers = ["https://223.5.5.5/dns-query", "https://doh.pub/dns-query"];
const foreignNameservers = ["https://77.88.8.8/dns-query", "https://1.1.1.1/dns-query", "https://8.8.4.4/dns-query#ecs=1.1.1.1/24&ecs-override=true", "https://208.67.222.222/dns-query#ecs=1.1.1.1/24&ecs-override=true", "https://9.9.9.9/dns-query"];
const dnsConfig = {
    "enable": true,
    "listen": "0.0.0.0:1053",
    "secret": "K!c*ow9!@BgS!6Kw9r",
    "ipv6": true,
    "prefer-h3": true,
    "use-system-hosts": false,
    "cache-algorithm": "arc",
    "enhanced-mode": "fake-ip",
    "fake-ip-range": "198.18.0.1/16",
    "fake-ip-filter": [
        "+.lan",
        "+.local",
        "+.msftconnecttest.com",
        "+.msftncsi.com",
        "localhost.ptlogin2.qq.com",
        "localhost.sec.qq.com",
        "localhost.work.weixin.qq.com"
    ],
    "default-nameserver": [
        "223.5.5.5",
        "1.2.4.8"
    ],
    "nameserver": [...foreignNameservers],
    "proxy-server-nameserver": [...domesticNameservers],
    "respect-rules": true,
    "nameserver-policy": {
        "geosite:private,cn": domesticNameservers
    }
};

function getNodeNames(allProxyNames, regionKeys = []) {
    if (!regionKeys || regionKeys.length === 0) return allProxyNames;
    const matchedProxies = new Set();
    for (const key of regionKeys) {
        const region = REGIONS[key];
        if (region && region.regex) {
            allProxyNames.forEach(proxyName => {
                if (region.regex.test(proxyName)) matchedProxies.add(proxyName);
            });
        }
    }
    return Array.from(matchedProxies);
}

// ===================================================================================
// 6. 程序主入口 (无需修改)
// ===================================================================================
function main(config) {
    if ((config?.proxies?.length ?? 0) === 0 && (typeof config?.["proxy-providers"] === "object" ? Object.keys(config["proxy-providers"]).length : 0) === 0) {
        throw new Error("配置文件中未找到任何代理");
    }

    const initialProxies = [...(config.proxies || []), ...chainLandingProxies];
    config.proxies = initialProxies.filter(p => !EXCLUDED_KEYWORDS.some(keyword => p.name.includes(keyword)));
    const allProxyNames = config.proxies.map(proxy => proxy.name);
    const chainProxiesName = chainLandingProxies.map(proxy => proxy.name);

    const manualRegionGroups = [];
    const autoRegionGroups = [];
    Object.keys(REGIONS).forEach(key => {
        const nodesInRegion = getNodeNames(allProxyNames, [key]);
        if (nodesInRegion.length > 0) {
            manualRegionGroups.push({
                ...groupBaseOption,
                name: `${key}-手动选择`,
                type: 'select',
                proxies: nodesInRegion,
                icon: REGIONS[key].icon
            });
            autoRegionGroups.push({
                ...groupBaseOption,
                name: `${key}-自动选择`,
                type: 'url-test',
                url: "http://www.gstatic.com/generate_204",
                interval: 300,
                tolerance: 50,
                proxies: nodesInRegion,
                hidden: true
            });
        }
    });
    const allManualRegionGroupNames = manualRegionGroups.map(g => g.name);

    const dynamicServiceGroups = [];
    const dynamicRuleProviders = {};
    const dynamicRules = [];
    for (const serviceKey in ENABLED_SERVICES) {
        const service = ENABLED_SERVICES[serviceKey];
        if (service.enabled) {
            const availableProxies = ['节点选择'];
            if (service.allowDirect) {
                availableProxies.unshift('DIRECT');
            }
            let hasSpecificRegions = service.regions && service.regions.length > 0;
            let optionsFound = false;
            if (hasSpecificRegions) {
                const regionalNodes = getNodeNames(allProxyNames, service.regions);
                service.regions.forEach(key => {
                    if (allManualRegionGroupNames.includes(`${key}-手动选择`)) {
                        availableProxies.push(`${key}-手动选择`);
                        optionsFound = true;
                    }
                });
                if (regionalNodes.length > 0) {
                    availableProxies.push(...regionalNodes);
                    optionsFound = true;
                }
                if (!optionsFound) {
                    hasSpecificRegions = false;
                }
            }
            if (!hasSpecificRegions) {
                availableProxies.push('手动选择', ...allProxyNames);
            }
            dynamicServiceGroups.push({
                ...groupBaseOption,
                name: service.groupName,
                type: 'select',
                proxies: [...new Set(availableProxies)],
                icon: service.icon
            });
            dynamicRuleProviders[service.rule.providerKey] = {
                ...ruleProviderCommon,
                behavior: 'classical',
                url: service.rule.url,
                path: `./ruleset/generated/${service.rule.providerKey}.yaml`
            };
            let ruleString = `RULE-SET,${service.rule.providerKey},${service.groupName}`;
            if (service.rule.options) {
                ruleString += `,${service.rule.options}`;
            }
            dynamicRules.push(ruleString);
        }
    }

    const allAutoRegionGroupNames = autoRegionGroups.map(g => g.name);
    const nodeSelectionProxies = ["手动选择", "延迟选优", "故障转移"];
    if (chainLandingProxies && chainLandingProxies.length > 0) {
        nodeSelectionProxies.push(chainLandingName);
    }
    nodeSelectionProxies.push(...allManualRegionGroupNames);

    let baseProxyGroups = [
        {
            ...groupBaseOption,
            name: "节点选择",
            type: "select",
            proxies: nodeSelectionProxies,
            icon: "https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/adjust.svg"
        },
        {
            ...groupBaseOption,
            name: "手动选择",
            type: "select",
            proxies: allProxyNames,
            icon: "https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/link.svg"
        },
        {
            ...groupBaseOption,
            name: "延迟选优",
            type: "url-test",
            tolerance: 50,
            proxies: allProxyNames,
            icon: "https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/speed.svg"
        },
        {
            ...groupBaseOption,
            name: "故障转移",
            type: "fallback",
            proxies: allProxyNames,
            icon: "https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/ambulance.svg"
        },
        {
            ...groupBaseOption,
            name: "广告过滤",
            type: "select",
            proxies: ["REJECT", "DIRECT"],
            icon: "https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/bug.svg"
        },
        {
            ...groupBaseOption,
            name: "漏网之鱼",
            type: "select",
            proxies: ["节点选择", "DIRECT", ...allAutoRegionGroupNames],
            icon: "https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/fish.svg"
        },
    ];

    if (chainLandingProxies && chainLandingProxies.length > 0) {
        baseProxyGroups.push(
            {
                ...groupBaseOption,
                name: chainLandingName,
                type: "select",
                proxies: chainProxiesName,
                icon: "https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/adjust.svg"
            },
            {
                ...groupBaseOption,
                name: chainTransitName,
                type: "select",
                proxies: allProxyNames.filter(p => !chainProxiesName.includes(p)),
                icon: "https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/adjust.svg"
            }
        );
    }
    
    config["proxy-groups"] = [...baseProxyGroups, ...dynamicServiceGroups, ...manualRegionGroups, ...autoRegionGroups];
    config["rule-providers"] = {...staticRuleProviders, ...dynamicRuleProviders};
    config["rules"] = [...staticRules.top, ...dynamicRules, ...staticRules.bottom];
    config["dns"] = dnsConfig;
    config["proxies"].forEach(proxy => {
        proxy.udp = true;
    });

    return config;
}