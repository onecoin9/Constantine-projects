# modify_xml.py
import sys
import xml.etree.ElementTree as ET

def modify_xml_attribute(xml_file, node_path, attribute_name, new_value):
    # 解析 XML 文件
    tree = ET.parse(xml_file)
    root = tree.getroot()

    # 找到指定的节点
    for node in root.findall(node_path):
        # 修改节点的属性值
        node.set(attribute_name, new_value)
        print(f"Attribute '{attribute_name}' of {node_path} modified to '{new_value}'.")

    # 保存修改回文件
    tree.write(xml_file, encoding='utf-8', xml_declaration=True)

if __name__ == "__main__":
    if len(sys.argv) == 5:
        _, xml_file, node_path, attribute_name, new_value = sys.argv
        modify_xml_attribute(xml_file, node_path, attribute_name, new_value)
    else:
        print("Usage: python modify_xml.py <xml_file> <node_path> <attribute_name> <new_value>")