import sys
import os
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QLabel,
    QPushButton, QSizePolicy, QMenuBar, QMenu, QAction, QMessageBox, QScrollArea
)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QCursor, QPixmap


class DropZone(QLabel):
    def __init__(self, on_folder_dropped):
        super().__init__('Drop Folder With .bmp Images Here')
        self.setAcceptDrops(True)
        self.setAlignment(Qt.AlignCenter)
        self.setStyleSheet("border: 2px dashed #888; padding: 40px; color: #333333;")
        self.on_folder_dropped = on_folder_dropped

    def dragEnterEvent(self, event):
        if event.mimeData().hasUrls():
            event.acceptProposedAction()
        else:
            event.ignore()

    def dropEvent(self, event):
        for url in event.mimeData().urls():
            path = url.toLocalFile()
            if os.path.isdir(path):
                self.on_folder_dropped(path)
                break
        event.acceptProposedAction()


class ProcessImagesApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Process Images Tool")
        self.setStyleSheet("background-color: #f7efdf;")
        self.resize(900, 600)

        self.layout = QVBoxLayout()
        self.setLayout(self.layout)

        self.last_folder_path = None
        self.original_pixmap = None

        # Menu bar
        menu_bar = QMenuBar(self)
        self.layout.setMenuBar(menu_bar)

        # About menu
        about_menu = QMenu("Acerca de", self)
        menu_bar.addMenu(about_menu)

        team_action = QAction("Equipo", self)
        team_action.triggered.connect(self.show_team_info)
        about_menu.addAction(team_action)

        self.drop_zone = DropZone(self.process_folder)

        self.filename_label = QLabel("Folder Ready To be Processed")
        self.filename_label.setAlignment(Qt.AlignCenter)
        self.filename_label.setStyleSheet("font-size: 14px; color: #000000; padding: 5px;")
        self.filename_label.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.filename_label.setFixedHeight(40)
        self.filename_label.setVisible(False)

        self.run_image_processing_button = QPushButton("Process Images")
        self.run_image_processing_button.clicked.connect(
            lambda: self.process_images(self.last_folder_path)
        )
        self.run_image_processing_button.setStyleSheet("""
            QPushButton {
                background-color: #a1887f;
                color: white;
                border-radius: 6px;
                padding: 8px 16px;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: #8d6e63;
            }
        """)
        self.run_image_processing_button.setCursor(QCursor(Qt.PointingHandCursor))
        self.run_image_processing_button.setVisible(False)

        self.completion_label = QLabel("Processing complete. Check output folder at ./output")
        self.completion_label.setAlignment(Qt.AlignCenter)
        self.completion_label.setStyleSheet("font-size: 14px; color: black; padding: 5px;")
        self.completion_label.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.completion_label.setFixedHeight(30)
        self.completion_label.setVisible(False)

        self.layout.addWidget(self.drop_zone)
        self.layout.addWidget(self.filename_label)
        self.layout.addWidget(self.completion_label)
        self.layout.addWidget(self.run_image_processing_button)

    def process_folder(self, folderPath):
        self.last_folder_path = folderPath
        self.run_image_processing_button.setVisible(True)
        self.filename_label.setVisible(True)
        self.completion_label.setVisible(False)

        bmp_images = [f for f in os.listdir(folderPath) if f.lower().endswith(".bmp")]
        if bmp_images:
            output_text = "Images ready to process\n"
        else:
            output_text = "No .bmp images found in the folder."

        self.filename_label.setText(output_text)

    def process_images(self, folderPath):
        self.completion_label.setVisible(True)
        self.filename_label.setVisible(False)
        self.drop_zone.setVisible(False)

        if hasattr(self, "text_scroll_area"):
            self.text_scroll_area.deleteLater()
        if hasattr(self, "text_display_label"):
            self.text_display_label.deleteLater()
        if hasattr(self, "image_scroll_area"):
            self.image_scroll_area.deleteLater()

        base_dir = os.path.dirname(os.path.abspath(__file__))
        txt_files = [f for f in os.listdir(base_dir) if f.lower().endswith(".txt")]

        if txt_files:
            txt_file_path = os.path.join(base_dir, txt_files[0])
            with open(txt_file_path, 'r') as file:
                file_content = file.read()

            self.text_display_label = QLabel(file_content)
            self.text_display_label.setAlignment(Qt.AlignTop)
            self.text_display_label.setStyleSheet("font-size: 14px; color: #365b6d; padding: 5px;")
            self.text_display_label.setWordWrap(True)

            self.text_scroll_area = QScrollArea()
            self.text_scroll_area.setWidgetResizable(True)
            self.text_scroll_area.setStyleSheet("background-color: #f7efdf; border: none;")
            self.text_scroll_area.setWidget(self.text_display_label)
            self.text_scroll_area.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

            self.layout.insertWidget(
                self.layout.indexOf(self.run_image_processing_button),
                self.text_scroll_area,
                stretch=1
            )
        else:
            self.text_display_label = QLabel("No .txt file found next to gui.py")
            self.text_display_label.setAlignment(Qt.AlignCenter)
            self.text_display_label.setStyleSheet("font-size: 14px; color: black; padding: 5px;")
            self.layout.insertWidget(
                self.layout.indexOf(self.run_image_processing_button),
                self.text_display_label
            )

        # Load and show image
        image_path = os.path.join(base_dir, "images", "prices.png")
        if os.path.exists(image_path):
            self.original_pixmap = QPixmap(image_path)
            if self.original_pixmap.isNull():
                self.image_label = QLabel("Failed to load image.")
            else:
                self.image_label = QLabel()
                self.image_label.setAlignment(Qt.AlignCenter)
                self.image_label.setScaledContents(False)
                self.image_label.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)

                self.update_image_pixmap()

                self.image_scroll_area = QScrollArea()
                self.image_scroll_area.setWidgetResizable(True)
                self.image_scroll_area.setStyleSheet("background-color: #f7efdf; border: none;")
                self.image_scroll_area.setWidget(self.image_label)
                self.image_scroll_area.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
                self.image_scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
                self.image_scroll_area.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)

                self.layout.insertWidget(
                    self.layout.indexOf(self.run_image_processing_button),
                    self.image_scroll_area,
                    stretch=2
                )
        else:
            self.image_label = QLabel("Image not found at: " + image_path)
            self.image_label.setAlignment(Qt.AlignCenter)
            self.layout.insertWidget(
                self.layout.indexOf(self.run_image_processing_button),
                self.image_label
            )

    def update_image_pixmap(self):
        if self.original_pixmap and hasattr(self, "image_label"):
            container_width = self.width() - 40  # margin buffer
            scaled_pixmap = self.original_pixmap.scaledToWidth(container_width, Qt.SmoothTransformation)
            self.image_label.setPixmap(scaled_pixmap)

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self.update_image_pixmap()

    def show_team_info(self):
        msg = QMessageBox(self)
        msg.setWindowTitle("Equipo de Desarrollo")
        msg.setTextFormat(Qt.RichText)
        msg.setText(
            "<div style='color: black; font-size: 12px;'>"
            "Integrantes del equipo:<br>"
            "- Adrián Moras Acuña - A01552359<br>"
            "- André Rossell Manrique - A01736035<br>"
            "- Manuel Zepeda Del Río - A01733305"
            "</div>"
        )
        msg.setStyleSheet("""
            QMessageBox {
                background-color: #f7efdf;
            }
            QPushButton {
                background-color: #a1887f;
                color: white;
                border-radius: 6px;
                padding: 8px 16px;
                font-size: 14px;
                min-width: 80px;
            }
            QPushButton:hover {
                background-color: #8d6e63;
            }
        """)
        msg.exec_()


def run_cli_mode(file_path):
    with open(file_path, 'r') as f:
        print(f.read())


if __name__ == "__main__":
    if len(sys.argv) > 1:
        run_cli_mode(sys.argv[1])
    else:
        app = QApplication(sys.argv)
        window = ProcessImagesApp()
        window.show()
        sys.exit(app.exec_())
