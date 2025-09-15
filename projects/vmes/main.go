package main

import (
	"encoding/xml"
	"fmt"
	"io/ioutil"
	"net/http"

	"github.com/foomo/soap"
	"github.com/gin-gonic/gin"
	"gopkg.in/yaml.v2"
)

const (
	VERSION = "V1.0.8_230915"
)

//yaml文件内容影射的结构体，注意结构体成员要大写开头
type ServerConfig struct {
	ListenPort          string `yaml:"ListenPort"`
	LoginUrl            string `yaml:"LoginUrl"`
	GetTokenUrl         string `yaml:"GetTokenUrl"`
	GetLocationUrl      string `yaml:"GetLocationUrl"`
	GetWorkOrderUrl     string `yaml:"GetWorkOrderUrl"`
	SendTaskInfoUrl     string `yaml:"SendTaskInfoUrl"`
	SendAlarmInfoUrl    string `yaml:"SendAlarmInfoUrl"`
	SendOpeCallUrl      string `yaml:"SendOpeCallUrl"`
	SendProgInfoUrl     string `yaml:"SendProgInfoUrl"`
	SendProgResultUrl   string `yaml:"SendProgResultUrl"`
	SendReortUrl        string `yaml:"SendReportUrl"`
	GetSnInfoUrl        string `yaml:"GetSnInfoUrl"`
	SOAPListenPort      string `yaml:"SOAPListenPort"`
	SOAPUrl             string `yaml:"SOAPUrl"`
	SOAPAction          string `yaml:"SOAPAction"`
	SOAPBodyContentType string `yaml:"SOAPBodyContentType"`
}

// SOAPRequest a simple request
type SOAPRequest struct {
	XMLName  xml.Name
	PassCnt  string
	FailCnt  string
	TotalCnt string
}

// SOAPResponse a simple response
type SOAPResponse struct {
	PassCnt  string
	FailCnt  string
	TotalCnt string
}

var server_config ServerConfig //定义一个结构体变量

func main() {
	fmt.Println("Current Version: " + VERSION)

	//读取yaml文件到缓存中
	config, err := ioutil.ReadFile("./config.yaml")
	if err != nil {
		fmt.Print(err)
	}
	//yaml文件内容影射到结构体中
	err1 := yaml.Unmarshal(config, &server_config)
	if err1 != nil {
		fmt.Println("error")
		return
	}

	if server_config.GetTokenUrl == "" {
		fmt.Println("没有找到GetTokenUrl，请使用最新的配置文件config.yaml")
		return
	}
	if server_config.SendOpeCallUrl == "" {
		fmt.Println("没有找到SendOpeCallUrl，请使用最新的配置文件config.yaml")
		return
	}
	if server_config.ListenPort == "" {
		fmt.Println("没有找到ListenPort，请使用最新的配置文件config.yaml")
		return
	}
	if server_config.SOAPListenPort == "" {
		fmt.Println("没有找到SoapListenPort，请使用最新的配置文件config.yaml")
		return
	}
	if server_config.GetSnInfoUrl == "" {
		fmt.Println("没有找到GetSnInfoUrl，请使用最新的配置文件config.yaml")
		return
	}
	fmt.Println(server_config.SOAPListenPort)
	//gin.SetMode(gin.ReleaseMode)
	r := gin.Default()
	r.GET("/download/TestEMMC.tar.gz", func(c *gin.Context) {
		c.File("TestEMMC.tar.gz")
	})
	r.GET("/download/Test.tar.gz", func(c *gin.Context) {
		c.File("Test.tar.gz")
	})
	r.POST(server_config.LoginUrl, func(c *gin.Context) {
		recv_body := make([]byte, 2048)
		n, _ := c.Request.Body.Read(recv_body)
		fmt.Println(string(recv_body[0:n]))

		login_data, err := ioutil.ReadFile("./login.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(login_data))
	})
	r.GET(server_config.LoginUrl, func(c *gin.Context) {
		login_data, err := ioutil.ReadFile("./login.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(login_data))
	})
	r.POST(server_config.GetTokenUrl, func(c *gin.Context) {
		recv_body := make([]byte, 2048)
		n, _ := c.Request.Body.Read(recv_body)
		fmt.Println(string(recv_body[0:n]))

		token_data, err := ioutil.ReadFile("./token.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(token_data))
	})
	r.GET(server_config.GetTokenUrl, func(c *gin.Context) {
		token_data, err := ioutil.ReadFile("./token.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(token_data))
	})
	r.POST(server_config.GetLocationUrl, func(c *gin.Context) {
		recv_body := make([]byte, 2048)
		n, _ := c.Request.Body.Read(recv_body)
		fmt.Println(string(recv_body[0:n]))

		location_data, err := ioutil.ReadFile("./location.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(location_data))
	})
	r.GET(server_config.GetLocationUrl, func(c *gin.Context) {
		location_data, err := ioutil.ReadFile("./location.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(location_data))
	})
	r.POST(server_config.GetWorkOrderUrl, func(c *gin.Context) {
		recv_body := make([]byte, 2048)
		n, _ := c.Request.Body.Read(recv_body)
		fmt.Println(string(recv_body[0:n]))

		order_data, err := ioutil.ReadFile("./order.json")
		if err != nil {
			fmt.Print(err)
		}

		fmt.Println(string(order_data))
		c.Writer.WriteString(string(order_data))
	})
	r.GET(server_config.GetWorkOrderUrl, func(c *gin.Context) {
		order_data, err := ioutil.ReadFile("./order.json")
		if err != nil {
			fmt.Print(err)
		}

		fmt.Println(string(order_data))
		c.Writer.WriteString(string(order_data))
	})
	r.POST(server_config.SendTaskInfoUrl, func(c *gin.Context) {
		recv_body := make([]byte, 2048)
		n, _ := c.Request.Body.Read(recv_body)
		fmt.Println(string(recv_body[0:n]))

		success_data, err := ioutil.ReadFile("./success.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(success_data))
	})
	r.GET(server_config.SendTaskInfoUrl, func(c *gin.Context) {
		success_data, err := ioutil.ReadFile("./success.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(success_data))
	})
	r.POST(server_config.SendAlarmInfoUrl, func(c *gin.Context) {
		recv_body := make([]byte, 2048)
		n, _ := c.Request.Body.Read(recv_body)
		fmt.Println(string(recv_body[0:n]))

		success_data, err := ioutil.ReadFile("./success.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(success_data))
	})
	r.GET(server_config.SendAlarmInfoUrl, func(c *gin.Context) {
		success_data, err := ioutil.ReadFile("./success.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(success_data))
	})
	r.POST(server_config.SendOpeCallUrl, func(c *gin.Context) {
		recv_body := make([]byte, 2048)
		n, _ := c.Request.Body.Read(recv_body)
		fmt.Println(string(recv_body[0:n]))

		success_data, err := ioutil.ReadFile("./success.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(success_data))
	})
	r.GET(server_config.SendOpeCallUrl, func(c *gin.Context) {
		success_data, err := ioutil.ReadFile("./success.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(success_data))
	})
	r.POST(server_config.SendProgInfoUrl, func(c *gin.Context) {
		recv_body := make([]byte, 2048)
		n, _ := c.Request.Body.Read(recv_body)
		fmt.Println(string(recv_body[0:n]))

		success_data, err := ioutil.ReadFile("./success.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(success_data))
	})
	r.GET(server_config.SendProgInfoUrl, func(c *gin.Context) {
		success_data, err := ioutil.ReadFile("./success.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(success_data))
	})
	r.POST(server_config.SendProgResultUrl, func(c *gin.Context) {
		recv_body := make([]byte, 2048)
		n, _ := c.Request.Body.Read(recv_body)
		fmt.Println(string(recv_body[0:n]))

		success_data, err := ioutil.ReadFile("./success.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(success_data))
	})
	r.GET(server_config.SendProgResultUrl, func(c *gin.Context) {
		success_data, err := ioutil.ReadFile("./success.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(success_data))
	})
	r.POST(server_config.SendReortUrl, func(c *gin.Context) {
		recv_body := make([]byte, 2048)
		n, _ := c.Request.Body.Read(recv_body)
		fmt.Println(string(recv_body[0:n]))

		success_data, err := ioutil.ReadFile("./success.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(success_data))
	})
	r.GET(server_config.SendReortUrl, func(c *gin.Context) {

		success_data, err := ioutil.ReadFile("./success.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(success_data))
	})
	r.POST(server_config.GetSnInfoUrl, func(c *gin.Context) {
		recv_body := make([]byte, 2048)
		n, _ := c.Request.Body.Read(recv_body)
		fmt.Println(string(recv_body[0:n]))

		getsninfo_data, err := ioutil.ReadFile("./getsninfo.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(getsninfo_data))
	})
	r.GET(server_config.GetSnInfoUrl, func(c *gin.Context) {
		getsninfo_data, err := ioutil.ReadFile("./getsninfo.json")
		if err != nil {
			fmt.Print(err)
		}
		c.Writer.WriteString(string(getsninfo_data))
	})
	go func() {
		soapPath := server_config.SOAPUrl
		fmt.Println("Run soap Server...", soapPath)
		soap.Verbose = true
		soapServer := soap.NewServer()
		soapServer.RegisterHandler(
			// SOAPpath
			soapPath,
			// SOAPAction
			server_config.SOAPAction,
			// tagname of soap body content
			server_config.SOAPBodyContentType,
			// RequestFactoryFunc - give the server sth. to unmarshal the request into
			func() interface{} {
				return &SOAPRequest{}
			},
			// OperationHandlerFunc - do something
			func(request interface{}, w http.ResponseWriter, httpRequest *http.Request) (response interface{}, err error) {
				soapRequest := request.(*SOAPRequest)
				soapResponse := &SOAPResponse{
					PassCnt:  "Hello " + soapRequest.PassCnt,
					FailCnt:  "Hello " + soapRequest.FailCnt,
					TotalCnt: "Hello " + soapRequest.TotalCnt,
				}
				response = soapResponse
				return
			},
		)
		err := soapServer.ListenAndServe(":" + server_config.SOAPListenPort)
		fmt.Println("exiting with error", err)
	}()

	r.Run(":" + server_config.ListenPort)

}
