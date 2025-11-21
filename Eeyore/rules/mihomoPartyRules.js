/**
 * Mihomo Party 智能配置脚本
 *
 * @Version 1.0
 * @Description
 * 结合 Smart 内核 AI 选择与传统精细化分流的优势
 * - 支持 Smart 内核的智能节点选择
 * - 保留服务级别的精细化分流
 * - 自动转换 url-test/load-balance 为 smart 类型
 * - 模块化服务配置管理
 *
 * @Features
 * 1. Smart 内核智能选择 + 服务分组
 * 2. 区域节点自动分组
 * 3. 自定义规则支持
 * 4. 灵活的策略配置
 */

// ===================================================================================
// 1. Smart 内核配置
// ===================================================================================
const SMART_CONFIG = {
    enabled: true,                      // 是否启用 Smart 内核
    profileCollectorSize: 100,          // Smart 数据收集大小
    convertExistingGroups: true,        // 自动转换现有 url-test/load-balance
    useLightGBM: true,                  // 使用 LightGBM 算法
    collectData: false,                 // 是否收集训练数据
    strategy: 'sticky-sessions',        // 策略: sticky-sessions/rr
    createSmartGroups: true,            // 为每个服务创建 Smart 组
};

// ===================================================================================
// 2. 节点过滤配置
// ===================================================================================
const EXCLUDED_KEYWORDS = [
    '官网', '到期', '流量', '剩余', '时间', '重置', '订阅', '卡顿',
    'Kitty Network', 'kitty.su', 'TG频道', '仅供个人使用'
];

// ===================================================================================
// 3. 自定义规则配置
// ===================================================================================
const CustomizationRule = [
    "DOMAIN-SUFFIX,jetbrains.ai,节点选择",
    "PROCESS-NAME,tailscaled,DIRECT",
    "PROCESS-NAME,tailscaled.exe,DIRECT",
    "DOMAIN-SUFFIX,mcdn.bilivideo.com,REJECT",
    "DOMAIN-SUFFIX,mcdn.bilivideo.cn,REJECT",
    "DOMAIN-SUFFIX,szbdyd.com,REJECT",
];

// ===================================================================================
// 4. 区域配置
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
    GB: {
        name: '英国',
        regex: /英国|UK|GB|United Kingdom|Britain|🇬🇧/,
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/flags/gb.svg'
    },
};

// ===================================================================================
// 5. 服务配置 (支持 Smart 分组)
// ===================================================================================
const ENABLED_SERVICES = {
    'openai': {
        enabled: true,
        allowDirect: false,
        groupName: 'ChatGPT',
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/chatgpt.svg',
        regions: ['US'],                    // 优先地区
        useSmart: true,                     // 为此服务创建 Smart 组
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
        useSmart: true,
        rule: {
            providerKey: 'anthropic',
            url: 'https://raw.githubusercontent.com/MetaCubeX/meta-rules-dat/refs/heads/meta/geo/geosite/classical/anthropic.yaml'
        }
    },
    'telegram': {
        enabled: true,
        allowDirect: false,
        groupName: 'Telegram',
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/telegram.svg',
        regions: ['SG'],
        useSmart: true,
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
        useSmart: true,
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
        useSmart: true,
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
        useSmart: false,                    // Microsoft 使用传统选择
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
        useSmart: false,
        rule: {
            providerKey: 'apple',
            url: 'https://raw.githubusercontent.com/blackmatrix7/ios_rule_script/master/rule/Clash/Apple/Apple_Classical.yaml'
        }
    },
    'netflix': {
        enabled: true,
        allowDirect: false,
        groupName: 'Netflix',
        icon: 'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/netflix.svg',
        regions: ['HK', 'SG', 'JP', 'US'],
        useSmart: true,
        rule: {
            providerKey: 'netflix',
            url: 'https://raw.githubusercontent.com/blackmatrix7/ios_rule_script/master/rule/Clash/Netflix/Netflix_Classical.yaml'
        }
    },
};

// ===================================================================================
// 6. 静态配置
// ===================================================================================
const groupBaseOption = {
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
const foreignNameservers = ["https://1.1.1.1/dns-query", "https://8.8.4.4/dns-query"];

const dnsConfig = {
    "enable": true,
    "listen": "0.0.0.0:1053",
    "ipv6": true,
    "prefer-h3": true,
    "enhanced-mode": "fake-ip",
    "fake-ip-range": "198.18.0.1/16",
    "fake-ip-filter": ["+.lan", "+.local", "localhost.ptlogin2.qq.com"],
    "default-nameserver": ["223.5.5.5", "1.2.4.8"],
    "nameserver": [...foreignNameservers],
    "proxy-server-nameserver": [...domesticNameservers],
    "nameserver-policy": {
        "geosite:private,cn": domesticNameservers
    }
};

// ===================================================================================
// 7. 工具函数
// ===================================================================================

/**
 * 获取指定区域的节点名称
 */
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

/**
 * 创建 Smart 代理组
 */
function createSmartGroup(name, proxies, icon, policyPriority = '') {
    if (!SMART_CONFIG.enabled || !proxies || proxies.length === 0) {
        return null;
    }
    
    return {
        name: name,
        type: 'smart',
        'policy-priority': policyPriority,
        'use-lightgbm': SMART_CONFIG.useLightGBM,
        'collect-data': SMART_CONFIG.collectData,
        strategy: SMART_CONFIG.strategy,
        proxies: proxies,
        icon: icon,
        ...groupBaseOption
    };
}

/**
 * 转换现有代理组为 Smart 类型
 */
function convertToSmartGroups(config) {
    if (!SMART_CONFIG.convertExistingGroups || !config['proxy-groups']) {
        return { converted: false, nameMapping: new Map() };
    }

    const nameMapping = new Map();
    let converted = false;

    for (let i = 0; i < config['proxy-groups'].length; i++) {
        const group = config['proxy-groups'][i];
        if (group && group.type) {
            const groupType = group.type.toLowerCase();
            if (groupType === 'url-test' || groupType === 'load-balance') {
                console.log('[Mihomo Smart] Converting group:', group.name, 'from', group.type, 'to smart');
                
                const originalName = group.name;
                group.type = 'smart';
                
                // 添加后缀标识
                if (!group.name.includes('(Smart)')) {
                    group.name = group.name + ' (Smart)';
                    nameMapping.set(originalName, group.name);
                }
                
                // 配置 Smart 参数
                group['policy-priority'] = group['policy-priority'] || '';
                group['use-lightgbm'] = SMART_CONFIG.useLightGBM;
                group['collect-data'] = SMART_CONFIG.collectData;
                group.strategy = SMART_CONFIG.strategy;
                
                // 移除旧配置
                delete group.url;
                delete group.interval;
                delete group.tolerance;
                delete group.lazy;
                delete group['expected-status'];
                
                converted = true;
            }
        }
    }

    // 更新引用
    if (nameMapping.size > 0) {
        updateGroupReferences(config, nameMapping);
    }

    return { converted, nameMapping };
}

/**
 * 更新配置中的代理组引用
 */
function updateGroupReferences(config, nameMapping) {
    // 更新代理组中的 proxies 引用
    if (config['proxy-groups']) {
        config['proxy-groups'].forEach(group => {
            if (group && group.proxies && Array.isArray(group.proxies)) {
                group.proxies = group.proxies.map(proxyName => 
                    nameMapping.get(proxyName) || proxyName
                );
            }
        });
    }

    // 更新规则引用
    if (config.rules && Array.isArray(config.rules)) {
        config.rules = config.rules.map(rule => {
            if (typeof rule === 'string') {
                let updatedRule = rule;
                nameMapping.forEach((newName, oldName) => {
                    const regex = new RegExp(`\\b${oldName}\\b`, 'g');
                    updatedRule = updatedRule.replace(regex, newName);
                });
                return updatedRule;
            }
            return rule;
        });
    }
}

// ===================================================================================
// 8. 主函数
// ===================================================================================
function main(config) {
    try {
        console.log('[Mihomo Smart] Starting configuration...');

        // 验证配置
        if (!config || typeof config !== 'object') {
            throw new Error('Invalid config object');
        }

        if ((config?.proxies?.length ?? 0) === 0) {
            throw new Error('No proxies found in config');
        }

        // 设置 Smart 内核配置
        if (SMART_CONFIG.enabled) {
            if (!config.profile) config.profile = {};
            config.profile['smart-collector-size'] = SMART_CONFIG.profileCollectorSize;
            console.log('[Mihomo Smart] Smart kernel enabled with collector size:', SMART_CONFIG.profileCollectorSize);
        }

        // 转换现有代理组
        const { converted } = convertToSmartGroups(config);
        if (converted) {
            console.log('[Mihomo Smart] Existing groups converted, returning config');
            return config;
        }

        // 过滤节点
        config.proxies = config.proxies.filter(p => 
            !EXCLUDED_KEYWORDS.some(keyword => p.name.includes(keyword))
        );
        const allProxyNames = config.proxies.map(proxy => proxy.name);
        console.log('[Mihomo Smart] Total proxies after filtering:', allProxyNames.length);

        // 创建区域分组
        const manualRegionGroups = [];
        const smartRegionGroups = [];
        
        Object.keys(REGIONS).forEach(key => {
            const nodesInRegion = getNodeNames(allProxyNames, [key]);
            if (nodesInRegion.length > 0) {
                // 手动选择组
                manualRegionGroups.push({
                    ...groupBaseOption,
                    name: `${key}-手动选择`,
                    type: 'select',
                    proxies: nodesInRegion,
                    icon: REGIONS[key].icon
                });
                
                // Smart 智能组
                if (SMART_CONFIG.enabled && SMART_CONFIG.createSmartGroups) {
                    const smartGroup = createSmartGroup(
                        `${key}-智能选择`,
                        nodesInRegion,
                        REGIONS[key].icon
                    );
                    if (smartGroup) {
                        smartGroup.hidden = true;
                        smartRegionGroups.push(smartGroup);
                    }
                }
            }
        });

        const allManualRegionGroupNames = manualRegionGroups.map(g => g.name);
        const allSmartRegionGroupNames = smartRegionGroups.map(g => g.name);

        // 创建服务分组
        const serviceGroups = [];
        const dynamicRuleProviders = {};
        const dynamicRules = [];

        for (const serviceKey in ENABLED_SERVICES) {
            const service = ENABLED_SERVICES[serviceKey];
            if (!service.enabled) continue;

            const availableProxies = [];
            
            // 添加服务自己的选择组
            availableProxies.push(service.groupName);
            
            // 添加 DIRECT 选项
            if (service.allowDirect) {
                availableProxies.unshift('DIRECT');
            }

            // 获取该服务的节点
            let serviceProxies = [];
            if (service.regions && service.regions.length > 0) {
                serviceProxies = getNodeNames(allProxyNames, service.regions);
                
                // 添加区域组
                service.regions.forEach(key => {
                    if (allManualRegionGroupNames.includes(`${key}-手动选择`)) {
                        availableProxies.push(`${key}-手动选择`);
                    }
                    if (allSmartRegionGroupNames.includes(`${key}-智能选择`)) {
                        availableProxies.push(`${key}-智能选择`);
                    }
                });
            } else {
                serviceProxies = allProxyNames;
            }

            // 创建服务选择组
            const serviceSelectGroup = {
                ...groupBaseOption,
                name: service.groupName,
                type: 'select',
                proxies: availableProxies.length > 0 ? availableProxies : ['节点选择'],
                icon: service.icon
            };
            serviceGroups.push(serviceSelectGroup);

            // 创建 Smart 组 (如果启用)
            if (SMART_CONFIG.enabled && service.useSmart && serviceProxies.length > 0) {
                const smartGroup = createSmartGroup(
                    `${service.groupName}-智能`,
                    serviceProxies,
                    service.icon,
                    service.policyPriority || ''
                );
                if (smartGroup) {
                    serviceGroups.push(smartGroup);
                    // 将 Smart 组添加到选择组的第一个位置(DIRECT 之后)
                    const directIndex = serviceSelectGroup.proxies.indexOf('DIRECT');
                    const insertIndex = directIndex >= 0 ? directIndex + 1 : 0;
                    serviceSelectGroup.proxies.splice(insertIndex, 0, `${service.groupName}-智能`);
                }
            }

            // 添加规则
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

        // 创建基础代理组
        const nodeSelectionProxies = ["手动选择", "自动选择", ...allManualRegionGroupNames];
        
        // 创建全局 Smart 组
        let globalSmartGroup = null;
        if (SMART_CONFIG.enabled && SMART_CONFIG.createSmartGroups) {
            globalSmartGroup = createSmartGroup(
                '智能选择',
                allProxyNames,
                'https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/speed.svg'
            );
            if (globalSmartGroup) {
                nodeSelectionProxies.splice(1, 0, '智能选择');
            }
        }

        const baseProxyGroups = [
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
                name: "自动选择",
                type: "url-test",
                tolerance: 50,
                interval: 300,
                proxies: allProxyNames,
                icon: "https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/speed.svg"
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
                proxies: ["节点选择", "DIRECT", ...allSmartRegionGroupNames],
                icon: "https://fastly.jsdelivr.net/gh/clash-verge-rev/clash-verge-rev.github.io@main/docs/assets/icons/fish.svg"
            },
        ];

        // 插入全局 Smart 组
        if (globalSmartGroup) {
            baseProxyGroups.splice(3, 0, globalSmartGroup);
        }

        // 组装最终配置
        config['proxy-groups'] = [
            ...baseProxyGroups,
            ...serviceGroups,
            ...manualRegionGroups,
            ...smartRegionGroups
        ];

        config['rule-providers'] = { ...staticRuleProviders, ...dynamicRuleProviders };
        config['rules'] = [...staticRules.top, ...dynamicRules, ...staticRules.bottom];
        config['dns'] = dnsConfig;

        // 启用 UDP
        config.proxies.forEach(proxy => {
            proxy.udp = true;
        });

        console.log('[Mihomo Smart] Configuration completed successfully');
        console.log('[Mihomo Smart] Total proxy groups:', config['proxy-groups'].length);
        console.log('[Mihomo Smart] Smart groups created:', smartRegionGroups.length + (globalSmartGroup ? 1 : 0));
        
        return config;

    } catch (error) {
        console.error('[Mihomo Smart] Error:', error.message);
        console.error('[Mihomo Smart] Stack:', error.stack);
        return config;
    }
}
