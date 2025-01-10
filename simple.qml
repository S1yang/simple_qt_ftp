
import Network 1.0

    FtpUploader {
        id: ftpUploader  // 创建 FtpUploader 实例
    }

            // 上传图片按钮
            Button {
                text: ""
                width: parent.width * 0.4
                height: Screen.height * 0.08
                background: Rectangle {
                    color: "#FF6347"
                    radius: 5
                }
                contentItem: Text {
                    text: "上传图片"
                    color: "white"
                    font.bold: true
                    font.pointSize: Screen.width > 400 ? 14 : 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.fill: parent
                }
                onClicked: {
                    fileDialog.open()
                }

            }


            // 文件选择对话框
            FileDialog {
                id: fileDialog
                title: "选择图片文件"
                nameFilters: ["Images (*.jpg *.png)"]
                selectMultiple: true  // 允许多选文件
                onAccepted: {
                    let selectedFiles = fileDialog.fileUrls;
                    for (var i = 0; i < selectedFiles.length; i++) {
                        var fileUrl = selectedFiles[i]
                        var filePath = fileUrl.toString().replace("file:///", "")
                        var fileName = filePath.split('/').pop()
                        var ftpAddress = "ftp://" + ipAddress + ftpFolderPath + fileName

                        // 每次都 new 一个新的 FtpUploader，对应自己的 m_file 和 m_reply
                        var uploader = Qt.createQmlObject('import Network 1.0; FtpUploader {}', parent, "")
                        uploader.uploadToFtp(filePath, ftpAddress)
                    }
                }
            }
