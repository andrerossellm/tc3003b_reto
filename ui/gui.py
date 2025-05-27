import sys
import os
import subprocess
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QLabel,
    QPushButton, QSizePolicy, QMenuBar, QMenu, QAction, QMessageBox, QScrollArea,
    QProgressBar 
)
from PyQt5.QtCore import Qt, QProcess 
from PyQt5.QtGui import QCursor, QPixmap
import platform


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
        self.total_images_to_process = 0 

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

        # New: Progress Bar
        self.progress_bar = QProgressBar()
        self.progress_bar.setMinimum(0)
        self.progress_bar.setMaximum(100)
        self.progress_bar.setValue(0)
        self.progress_bar.setStyleSheet("""
            QProgressBar {
                border: 2px solid #a1887f;
                border-radius: 5px;
                background-color: #e0e0e0;
                text-align: center;
                color: #333333;
            }
            QProgressBar::chunk {
                background-color: #a1887f;
                width: 10px;
            }
        """)
        self.progress_bar.setVisible(False)

        self.run_image_processing_button = QPushButton("Process Images")
        self.run_image_processing_button.clicked.connect(
            lambda: self.execute_c_code(self.last_folder_path)
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
        self.layout.addWidget(self.progress_bar) # Add progress bar to layout
        self.layout.addWidget(self.completion_label)
        self.layout.addWidget(self.run_image_processing_button)

        self.c_process = QProcess(self)
        self.c_process.readyReadStandardOutput.connect(self.read_c_output)
        self.c_process.readyReadStandardError.connect(self.read_c_error)
        self.c_process.finished.connect(self.c_process_finished)

    def process_folder(self, folderPath):
        self.last_folder_path = folderPath
        self.run_image_processing_button.setVisible(True)
        self.filename_label.setVisible(True)
        self.completion_label.setVisible(False)
        self.progress_bar.setVisible(False) 

        bmp_images = [f for f in os.listdir(folderPath) if f.lower().endswith(".bmp")]
        self.total_images_to_process = len(bmp_images) 

        if bmp_images:
            output_text = "Images ready to process:\n" + "\n".join(bmp_images[:5])
            if len(bmp_images) > 5:
                output_text += "\n..."
            output_text += f"\nTotal: {len(bmp_images)} images."
        else:
            output_text = "No .bmp images found in the folder."

        self.filename_label.setText(output_text)

    def execute_c_code(self, folderPath):
            if not folderPath:
                QMessageBox.warning(self, "Warning", "Please drop a folder first.")
                return

            bmp_images = [f for f in os.listdir(folderPath) if f.lower().endswith(".bmp")]
            if not bmp_images:
                QMessageBox.warning(self, "Warning", "No .bmp images found in the selected folder.")
                return

            self.completion_label.setText("Processing images...")
            self.completion_label.setVisible(True)
            self.filename_label.setVisible(False)
            self.drop_zone.setVisible(False)
            self.run_image_processing_button.setVisible(False)  
            self.run_image_processing_button.setEnabled(False)
            self.progress_bar.setValue(0)
            self.progress_bar.setVisible(True) 

            arch = platform.machine()

            if arch == "x86_64":
                mpi_master_executable = "x86"
            elif arch == "aarch64":
                mpi_master_executable = "arm.out"
            else:
                raise RuntimeError(f"Unsupported architecture: {arch}")

            # mpi_slave_executable = "/home/youruser/mpi_execs/image_processor_intel" # Not used directly in Python
            # hosts_file = "ui/mpi_hosts" # Not used directly in Python

            if getattr(sys, 'frozen', False):
                base_dir = sys._MEIPASS
            else:
                base_dir = os.path.dirname(os.path.abspath(__file__))
            mpi_master_executable_path = os.path.join(base_dir, mpi_master_executable)

            output_dir = os.path.join(os.getcwd(), "output")
            os.makedirs(output_dir, exist_ok=True)

            try:
                command = ['mpirun', '-n', '4', mpi_master_executable_path, folderPath]
                self.c_process.start(command[0], command[1:])
                if not self.c_process.waitForStarted(5000): 
                        raise Exception(f"Failed to start MPI process: {self.c_process.errorString()}")

            except FileNotFoundError:
                QMessageBox.critical(self, "Error", "mpirun command not found. Make sure Open MPI is installed and in your PATH.")
                self.c_process_finished(1) 
            except Exception as e:
                QMessageBox.critical(self, "Error", f"An unexpected error occurred: {e}")
                self.c_process_finished(1) 

    def read_c_output(self):
        while self.c_process.canReadLine():
            line = str(self.c_process.readLine(), 'utf-8').strip()
            # print(f"C Output: {line}") # For debugging

            if line.startswith("PROGRESS:"):
                try:
                    parts = line.split(":")
                    if len(parts) > 1:
                        progress_data = parts[1].strip().split("/")
                        if len(progress_data) == 2:
                            processed = int(progress_data[0])
                            total = int(progress_data[1])
                            if total > 0:
                                percentage = int((processed / total) * 100)
                                self.progress_bar.setValue(percentage)
                                self.completion_label.setText(f"Processing images... {processed}/{total} ({percentage}%)")
                            else:
                                self.progress_bar.setValue(0)
                                self.completion_label.setText("Processing images...")
                except ValueError:
                    print(f"Could not parse progress line: {line}")
            else:
                # You can choose to display other C code output in a debug console or a log window
                pass

    def read_c_error(self):
        error_output = str(self.c_process.readAllStandardError(), 'utf-8').strip()
        if error_output:
            print(f"C Error: {error_output}")

    def c_process_finished(self, exit_code, exit_status=QProcess.NormalExit):
            if exit_code == 0:
                self.completion_label.setText("Processing complete. Check output folder at ./output")
                self.progress_bar.setValue(100)
            else:
                self.completion_label.setText("Processing failed. See console for details.")
                self.progress_bar.setValue(0) # Reset or indicate failure

            self.run_image_processing_button.setText("Restart")
            self.run_image_processing_button.setEnabled(True)
            self.run_image_processing_button.setVisible(True) 
            self.run_image_processing_button.clicked.disconnect()
            self.run_image_processing_button.clicked.connect(self.reset_ui_after_processing)

            base_dir = os.path.dirname(os.path.abspath(__file__))
            txt_files = [f for f in os.listdir(base_dir) if f.lower().endswith(".txt")]

            if hasattr(self, "text_scroll_area"):
                self.text_scroll_area.deleteLater()
            if hasattr(self, "text_display_label"):
                self.text_display_label.deleteLater()
            if hasattr(self, "image_scroll_area"):
                self.image_scroll_area.deleteLater()

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

    def reset_ui_after_processing(self):
        self.filename_label.setVisible(False)
        self.completion_label.setVisible(False)
        self.run_image_processing_button.setVisible(False)
        self.progress_bar.setVisible(False) 

        if hasattr(self, "text_scroll_area"):
            self.text_scroll_area.deleteLater()
            del self.text_scroll_area
        if hasattr(self, "text_display_label"):
            self.text_display_label.deleteLater()
            del self.text_display_label
        if hasattr(self, "image_scroll_area"):
            self.image_scroll_area.deleteLater()
            del self.image_scroll_area
        if hasattr(self, "image_label"):
            self.image_label.deleteLater()
            del self.image_label

        self.drop_zone.setVisible(True)
        self.drop_zone.setText('Drop Folder With .bmp Images Here')

        self.run_image_processing_button.setText("Process Images")
        self.run_image_processing_button.clicked.disconnect()
        self.run_image_processing_button.clicked.connect(
            lambda: self.execute_c_code(self.last_folder_path)
        )

    def update_image_pixmap(self):
        if self.original_pixmap and hasattr(self, "image_label"):
            container_width = self.width() - 40
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