package main

import (
	"github.com/gin-gonic/gin"
	"query-pdb-proxy/conf"
	"query-pdb-proxy/proxy"
)

var (
	R *gin.Engine
)

func main() {
	gin.SetMode(gin.ReleaseMode)
	R = gin.Default()
	proxy.InitRoute(R)
	R.Run("0.0.0.0:" + conf.Port)
}
