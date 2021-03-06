// Copyright (C) 2018 Baidu Inc. All rights reserved.

package com.baidu.acu.pie

import com.baidu.acu.pie.client.AsrClientFactory
import com.baidu.acu.pie.model.AsrConfig
import com.baidu.acu.pie.model.AsrProduct
import org.junit.Ignore
import org.junit.Test
import java.nio.file.Paths

/**
 * KotlinDemo
 *
 * @author Shu Lingjie(shulingjie@baidu.com)
 */
@Ignore
class KotlinDemo {
    @Test
    fun testSendFile() {
        val audioFilePath = "testaudio/bj8k.wav"

        val asrConfig = AsrConfig()
            .serverIp("127.0.0.1")
            .serverPort(80)
            .appName("simple demo")
            .product(AsrProduct.CUSTOMER_SERVICE)

        val asrClient = AsrClientFactory.buildClient(asrConfig)
        val results = asrClient.syncRecognize(Paths.get(audioFilePath))

        // don't forget to shutdown !!!
        asrClient.shutdown()

        for (result in results) {
            println(
                String.format(
                    AsrConfig.TITLE_FORMAT,
                    "serial_num",
                    "err_no",
                    "err_message",
                    "start_time",
                    "end_time",
                    "result"
                )
            )
            println(
                String.format(
                    AsrConfig.TITLE_FORMAT,
                    result.serialNum,
                    result.errorCode,
                    result.errorMessage,
                    result.startTime,
                    result.endTime,
                    result.result
                )
            )
        }
    }
}