  library(dplyr)
  
  testType<-"remote"
  count<-5
  minBufferPower<-22
  maxBufferPower<-27
  fileSize<-142
  
  meanDownloadTime<-c()
  bufferSizes<-c()
  
  data <- read.table(
    paste("dados/",
          fileSize,
          "MB/",
          testType,
          "ClientData-",
          count,
          "-", 
          minBufferPower, 
          "~", 
          maxBufferPower,
          ".txt",  
          sep = ""),
    header = TRUE)
  
  for(i in c(minBufferPower:maxBufferPower)){
    meanDownloadTime <- c(meanDownloadTime, 
                          mean(dplyr::filter(data, 
                                             bufferSize == 2**i)$downloadTime)/1000000)
    bufferSizes <- c(bufferSizes, paste("2^",i, sep = ""))
  }
  
  barplot(meanDownloadTime, main=paste("Teste", testType, fileSize, "MB"),names.arg=bufferSizes, ylab="Tempo de Download (s)", xlab="Buffer Size (Bytes)")