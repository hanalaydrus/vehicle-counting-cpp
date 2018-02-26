// Model.cpp

#include "Model.h"

Model::Model(){

}


vector< map<string, boost::variant<int, string>> > Model::getCameras() {
	// Input : -
	// Output : Array of Map
    vector< map<string, boost::variant<int, string>> > cameras;
    
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "root");
		/* Connect to the MySQL  database */
		con->setSchema("camera");
	  
		stmt = con->createStatement();
		res = stmt->executeQuery("SELECT * FROM `camera`");
		while (res->next()) {
          /* Access column data by alias or column name */
            map<string, boost::variant<int, string>> data;
            data["url"] = res->getString("url");

            cameras.resize((res->getInt("camera_id"))+1);

            cameras.at(res->getInt("camera_id")) = data;
        }
        // configuration
        con->setSchema("volume");
        
        stmt = con->createStatement();
        res = stmt->executeQuery("SELECT * FROM `configuration`");
        while (res->next()) {
        /* Access column data by alias or column name */
            map<string, boost::variant<int, string>> data;
            data = cameras[res->getInt("camera_id")];

            data["x0"] = res->getInt("crossing_line_x0");
            data["y0"] = res->getInt("crossing_line_y0");
            data["x1"] = res->getInt("crossing_line_x1");
            data["y1"] = res->getInt("crossing_line_y1");

            cameras.at(res->getInt("camera_id")) = data;
        }

		delete res;
		delete stmt;
		delete con;
	  
	  } catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	  }
	  
	  return cameras;
}

void Model::storeVolumeData(int camera_id, int volume_size) {
	// Input : camera_id, volume_size
	// Output : -
    bool isExist = false;
    int volume_history_id, current_volume_size;
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "root");
		/* Connect to the MySQL  database */
		con->setSchema("volume");
	  
        stmt = con->createStatement();
        ostringstream query;
        query << "SELECT * FROM `volume_history` WHERE DATE(`date_time`) = CURDATE() AND `camera_id` = " << camera_id;
		res = stmt->executeQuery(query.str());
        
        while (res->next()) {
          /* Access column data by alias or column name */
            isExist = true;
            volume_history_id = res->getInt(1);
            current_volume_size = res->getInt("volume_size");
        }

        if (isExist && (current_volume_size != volume_size)) {
            stmt = con->createStatement();
            ostringstream query;
            query << "UPDATE `volume_history` SET `date_time` = CURRENT_TIMESTAMP, `volume_size` = " << volume_size << " WHERE `volume_history_id` = " << volume_history_id;
            stmt->execute(query.str());
        } else if (!isExist) {
            stmt = con->createStatement();
            ostringstream query;
            query << "INSERT INTO `volume_history` (`camera_id`, `volume_size`) VALUES (" << camera_id << ", " << volume_size << ")";
            stmt->execute(query.str());
        }

		delete res;
		delete stmt;
		delete con;
	  
	  } catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	  }
}

vector<boost::variant<int, string>> Model::getVolumeByID(int camera_id) {
	// Input : camera_id
	// Output : time, volume
	vector<boost::variant<int, string>> response;
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "root");
	  
		/* Connect to the MySQL test database */
		con->setSchema("volume");
	  
		stmt = con->createStatement();
		ostringstream query;
		query << "SELECT * FROM `volume_history` WHERE DATE(`date_time`) = CURDATE() AND `camera_id` = " << camera_id;
		res = stmt->executeQuery(query.str());
		while (res->next()) {
			response.push_back(res->getString("date_time"));
			// cout << "count mysql : " << res->getInt("volume_size") << ", camera_id: " << camera_id << endl;
			response.push_back(res->getInt("volume_size"));
		}
		delete res;
		delete stmt;
		delete con;
	  
	  } catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	  }

	  return response;
}

float Model::getPercentage(int camera_id, string date_time, int volume_size){

	float percentage;
	int volume_normal_size;

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "root");
	  
		/* Connect to the MySQL test database */
		con->setSchema("volume");
	  
		stmt = con->createStatement();
		ostringstream query;
		query << "SELECT * FROM `volume_normal` WHERE `camera_id` = " << camera_id << " ORDER BY ABS(`time` - TIME('" << date_time << "')) LIMIT 1";
		res = stmt->executeQuery(query.str());
		while (res->next()) {
			volume_normal_size = res->getInt("volume_normal_size");
		}
		delete res;
		delete stmt;
		delete con;
	  
	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
	// cout << "volume_size: " << volume_size << ", volume_normal_size" << volume_normal_size << endl; 
	percentage = ((( (float)volume_size - (float)volume_normal_size ) / (float)volume_normal_size) * 100);

	return percentage;
}