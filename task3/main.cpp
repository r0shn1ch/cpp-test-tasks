#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <iconv.h>
#include <curl/curl.h>
#include "tinyxml2.h"

using namespace tinyxml2;
using namespace std;

const string BASE_URL = "https://www.cbr.ru/scripts/XML_dynamic.asp?";
const string VAL_LIST_URL = "https://www.cbr.ru/scripts/XML_daily.asp";

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

string DownloadXML(const string& url) {
    CURL* curl = curl_easy_init();
    string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

string ConvertEncoding(const string& input, const char* fromEncoding, const char* toEncoding) {
    iconv_t cd = iconv_open(toEncoding, fromEncoding);
    if (cd == (iconv_t)-1) return {};
    size_t inBytesLeft = input.size();
    size_t outBytesLeft = input.size() * 4;
    string output(outBytesLeft, '\0');
    char* inBuf = const_cast<char*>(input.c_str());
    char* outBuf = output.data();
    size_t result = iconv(cd, &inBuf, &inBytesLeft, &outBuf, &outBytesLeft);
    if (result == (size_t)-1) {
        iconv_close(cd);
        return {};
    }
    iconv_close(cd);
    output.resize(output.size() - outBytesLeft);
    return output;
}

string FormatDate(time_t t) {
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y", localtime(&t));
    return string(buffer);
}

struct RateRecord {
    string currencyName;
    string date;
    double rate;
};

int main() {
    time_t now = time(nullptr);
    time_t past = now - 90 * 86400;
    string date1 = FormatDate(past);
    string date2 = FormatDate(now);

    string xmlValRaw = DownloadXML(VAL_LIST_URL);
    string xmlVal = ConvertEncoding(xmlValRaw, "WINDOWS-1251", "UTF-8");

    XMLDocument doc;
    doc.Parse(xmlVal.c_str());
    map<string, string> valuteNames;
    XMLElement* root = doc.FirstChildElement("ValCurs");
    for (XMLElement* valute = root->FirstChildElement("Valute"); valute; valute = valute->NextSiblingElement("Valute")) {
        const char* id = valute->Attribute("ID");
        string name = valute->FirstChildElement("Name")->GetText();
        if (id) valuteNames[id] = name;
    }

    vector<RateRecord> allRates;

    for (const auto& [id, name] : valuteNames) {
        string url = BASE_URL + "date_req1=" + date1 + "&date_req2=" + date2 + "&VAL_NM_RQ=" + id;
        string xmlDataRaw = DownloadXML(url);
        string xmlData = ConvertEncoding(xmlDataRaw, "WINDOWS-1251", "UTF-8");

        XMLDocument xdoc;
        xdoc.Parse(xmlData.c_str());
        XMLElement* valCurs = xdoc.FirstChildElement("ValCurs");
        if (!valCurs) continue;

        for (XMLElement* record = valCurs->FirstChildElement("Record"); record; record = record->NextSiblingElement("Record")) {
            const char* dateAttr = record->Attribute("Date");
            XMLElement* valueEl = record->FirstChildElement("Value");
            XMLElement* nominalEl = record->FirstChildElement("Nominal");

            if (valueEl && dateAttr && nominalEl) {
                string valueStr = valueEl->GetText();
                string dateStr = dateAttr;
                string nominalStr = nominalEl->GetText();
                replace(valueStr.begin(), valueStr.end(), ',', '.');
                double value = stod(valueStr);
                int nominal = stoi(nominalStr);
                double rate = value / nominal;
                allRates.push_back({name, dateStr, rate});
            }
        }
    }

    if (allRates.empty()) return 1;

    auto minIt = min_element(allRates.begin(), allRates.end(), [](const RateRecord& a, const RateRecord& b) {
        return a.rate < b.rate;
    });
    auto maxIt = max_element(allRates.begin(), allRates.end(), [](const RateRecord& a, const RateRecord& b) {
        return a.rate < b.rate;
    });

    double total = 0.0;
    for (const auto& r : allRates) total += r.rate;
    double avg = total / allRates.size();

    cout << fixed << setprecision(4);
    cout << "Максимальный курс: " << maxIt->rate << " (" << maxIt->currencyName << ") — " << maxIt->date << endl;
    cout << "Минимальный курс: " << minIt->rate << " (" << minIt->currencyName << ") — " << minIt->date << endl;
    cout << "Средний курс за период по всем валютам: " << avg << endl;

    ofstream fout("exchange_rates.csv", ios::out | ios::binary);
    fout << "\xEF\xBB\xBF";
    fout << "Currency,Date,Rate\n";
    for (const auto& r : allRates) {
        fout << '"' << r.currencyName << "\","
             << r.date << ","
             << fixed << setprecision(4) << r.rate << "\n";
    }
    fout.close();

    return 0;
}
