const Fiber = require('fibers');
const {Builder, By, Key} = require('selenium-webdriver');
const Xlsx = require("xlsx");
const fs = require("fs");

class Webo {
    constructor() {
        this.parameters = function() { return []; };
        this.arguments = null;
    }

    version() {
        return "1.2"
    }

    start(func) {
        if (process.argv.indexOf("--info") != -1) {
            console.log("Group:" + global.group);
            console.log("Name:" + global.name);
            console.log("Version:" + global.version);
            console.log("Data:" + (global.data? global.data : ""));
            return;
        }
        
        if (process.argv.indexOf("--parameters") != -1) {
            console.log(JSON.stringify(this.parameters()));
            return;
        }

        var browser = process.argv[2];
        if (!browser)
            return;

        if (process.argv[3])
            this.arguments = JSON.parse(process.argv[3]);

        var parent = this;
        Fiber(function() {
            parent.log("Webo Started!");
            this.builder = new Builder()
            .forBrowser(browser);
        
            this.builder.getCapabilities()
                .setAcceptInsecureCerts(true);

            var fiber = Fiber.current;
            this.builder.build()
            .then(function(res) {
                parent.log("Success opening Browser...");
                parent.driver = res;
            }).finally(function() { fiber.run(); });
            Fiber.yield();

            if (!parent.driver)
                parent.warn("Failed opening Browser!");
            else
                func();

            parent.log("Webo Finished!")
        }).run();
    }

    load(url) {
        var fiber = Fiber.current;
        this.driver.get(url)
        .finally(function() { fiber.run(); });
        Fiber.yield();
    }

    url() {
        var result = null;
        var fiber = Fiber.current;
        this.driver.getCurrentUrl()
        .then(function(res) { result = res; })
        .finally(function() { fiber.run(); });
        Fiber.yield();
        return result;
    }

    findElement(css) {
        var result = null;
        var fiber = Fiber.current;
        this.driver.findElement(By.css(css))
        .then(function(res) { result = res; })
        .finally(function() { fiber.run(); });
        Fiber.yield();
        
        return new WeboElement(this.driver, result);
    }

    findElements(css) {
        var result = null;
        var fiber = Fiber.current;
        this.driver.findElements(By.css(css))
        .then(function(res) { result = res; })
        .finally(function() { fiber.run(); });
        Fiber.yield();

        if (!result)
            return null;

        var parent = this;
        var list = [];
        result.forEach(function(element) {
            list.push(new WeboElement(parent.driver, element));
        });

        return list;
    }

    alert() {
        var result = null;
        var fiber = Fiber.current;
        this.driver.switchTo().alert()
        .then(function(res) {
            result = res;
        }, function (error) {} )
        .finally(function() { fiber.run(); });
        Fiber.yield();

        return new WeboAlert(result);
    }

    waitDocumentReady() {
        while (true) {
            var state = this.executeScript("return document.readyState");
            if (state == "complete")
                break;

            this.sleep(200);
        }
    }

    waitForElement(css, timeout) {
        var waitTime = 500;
        var numTime = 0;
        while (true) {
            var elems = this.findElements(css);
            if (elems.length > 0)
                return true;
    
            this.sleep(waitTime);
            numTime += waitTime;
            if (numTime >= timeout)
                return false;
        };
    }
    
    waitForAttributeCondition(css, attrib, operator, value, timeout) {
        var waitTime = 500;
        var numTime = 0;
        while (true) {
            var elems = this.findElements(css);
            if (elems.length > 0) {
                var elem = this.findElement(css);
                var tempValue = elem.attribute(attrib);
                if (eval("'" + tempValue + "'" + operator + "'" + value + "'")) {
                    this.sleep(200);
                    return true;
                }
            }
    
            this.sleep(waitTime);
            numTime += waitTime;
            if (numTime >= timeout)
                return false;
        };
    }
    
    waitForInputCondition(css, operator, value, timeout) {
        var waitTime = 500;
        var numTime = 0;
        while (true) {
            var elems = this.findElements(css);
            if (elems.length > 0) {
                var elem = this.findElement(css);
                var tempValue = elem.value();
                if (eval("'" + tempValue + "'" + operator + "'" + value + "'")) {
                    this.sleep(200);
                    return true;
                }
            }
    
            this.sleep(waitTime);
            numTime += waitTime;
            if (numTime >= timeout)
                return false;
        };
    }

    waitForTextCondition(css, operator, value, timeout) {
        var waitTime = 500;
        var numTime = 0;
        while (true) {
            var elems = this.findElements(css);
            if (elems.length > 0) {
                var elem = this.findElement(css);
                var text = elem.text();
                if (eval("'" + text.trim() + "'" + operator + "'" + value + "'")) {
                    this.sleep(200);
                    return true;
                }
            }
    
            this.sleep(waitTime);
            numTime += waitTime;
            if (numTime >= timeout)
                return false;
        };
    }
    
    waitForAlert(timeout) {
        var waitTime = 500;
        var numTime = 0;
        while (true) {
            var tempAlert = this.alert();
            if (tempAlert.isValid()) {
                this.sleep(200);
                return true;
            }
    
            this.sleep(waitTime);
            numTime += waitTime;
            if (numTime >= timeout)
                return false;
        };
    }

    sleep(timeout) {
        var fiber = Fiber.current;
        this.driver.sleep(timeout)
        .finally(function() { fiber.run(); });
        Fiber.yield();
    }

    enableLog(filename) {
        this.logFile = fs.openSync(filename, "w+");
    }

    log(msg) {
        console.log(msg);
        if (!this.logFile)
            return;

        fs.writeSync(this.logFile, msg);
    }

    warn(msg) {
        console.warn(msg);
        if (!this.logFile)
            return;

        fs.writeSync(this.logFile, msg);
    }

    quit() {
        if (this.logFile) {
            fs.closeSync(this.logFile);
            this.logFile = null;
        }

        this.driver.quit();
    }
   
    executeScript(script, ...args) {
        var result = null;
        var fiber = Fiber.current;
        this.driver.executeScript(script, ...args)
        .then(function(res) { result = res; })
        .finally(function() { fiber.run(); });
        Fiber.yield();
        return result;
    }
};

class WeboElement {
    constructor (driver, element) {
        this.driver = driver;
        this.element = element;
    }

    sleep(ms) {
        var fiber = Fiber.current;
        setTimeout(function() {
            fiber.run();
        }, ms);
        Fiber.yield();
    }

    findElement(css) {
        var result = null;
        var fiber = Fiber.current;
        this.element.findElement(By.css(css))
        .then(function(res) { result = res; })
        .finally(function() { fiber.run(); });
        Fiber.yield();

        return result? new WeboElement(this.driver, result) : null;
    }

    findElements(css) {
        var result = null;
        var fiber = Fiber.current;
        this.element.findElements(By.css(css))
        .then(function(res) { result = res; })
        .finally(function() { fiber.run(); });
        Fiber.yield();

        if (!result)
            return null;

        var parent = this;
        var list = [];
        result.forEach(function(element) {
            list.push(new WeboElement(parent.driver, element));
        });

        return list;
    }

    tagName() {
        var result = null;
        var fiber = Fiber.current;
        this.element.getTagName()
        .then(function(res) { result = res; })
        .finally(function() { fiber.run(); });
        Fiber.yield();
        return result;
    }

    attribute(attrib) {
        var result = null;
        var fiber = Fiber.current;
        this.element.getAttribute(attrib)
        .then(function(res) { result = res; })
        .finally(function() { fiber.run(); });
        Fiber.yield();
        return result;
    }

    setAttribute(attrib, value) {
        this.executeScript("arguments[0].setAttribute(arguments[1], arguments[2]);", this.element, attrib, value);
    }

    value() {
        var name = this.tagName();
        if (name == "input" || name == "select")
            return this.attribute("value");
        else if (name == "textarea")
            return this.text();
    }

    setValue(value) {
        var name = this.tagName();
        if (name == "input") {
            var type = this.attribute("type");
            if (type == "file")
                this.sendKeys(value);
            else
                this.setAttribute("value", value);
        }
        else if (name == "select") {
            var options = this.findElements("option");
            var check = function(opt) {
                var _val = opt.attribute("value");
                if (_val == value)
                    opt.click();
            }
            options.forEach(check);
        }
        else if (name == "textarea")
            this.executeScript("arguments[0].innerText = arguments[1];", this.element, value);
        
        this.sleep(200);
    }
    
    text() {
        var result = null;
        var fiber = Fiber.current;
        this.element.getText()
        .then(function(res) { result = res; })
        .finally(function() { fiber.run(); });
        Fiber.yield();
        return result;
    }

    sendKeys(...args) {
        var fiber = Fiber.current;
        var element = this.element;
        args.forEach(function(key) {
            element.sendKeys(key)
            .finally(function() { fiber.run(); });
            Fiber.yield();
        });
        this.sleep(200);
    }

    click() {
        var fiber = Fiber.current;
        this.element.click()
        .finally(function() { fiber.run(); });
        Fiber.yield();
        this.sleep(200);
    }

    executeScript(script, ...args) {
        var result = null;
        var fiber = Fiber.current;
        this.driver.executeScript(script, ...args)
        .then(function(res) { result = res; })
        .finally(function() { fiber.run(); });
        Fiber.yield();
        return result;
    }
};

class WeboAlert {
    constructor(alert) {
        this.alert = alert;
    }
    
    isValid() {
        if (!this.alert)
            return false;

        return true;
    }

    accept() {
        if (!this.alert)
            return;

        this.alert.accept();
    }
    
    dismiss() {
        if (!this.alert)
            return;

        this.alert.dismiss();
    }
};

class WeboBook {
    constructor(filename) {
        this.load(filename);
    }

    load(filename) {
        this.workbook = Xlsx.readFile(filename);
    }
    
    sheetNames() {
        if (!this.workbook)
            return [];

        return this.workbook.SheetNames;
    }

    sheet(name) {
        if (!this.workbook)
            return null;

        if (!this.workbook.Sheets[name])
            return null;
        
        return new WeboSheet(this.workbook.Sheets[name]);
    }

    static encodeCell(column, row) {
        return this.encodeColumn(column) + this.encodeRow(row);
    }

    static encodeRow(row) {
        return Xlsx.utils.encode_row(row);
    }

    static encodeColumn(column) {
        return Xlsx.utils.encode_col(column);
    }
};

class WeboSheet {
    constructor(worksheet) {
        this.worksheet = worksheet;
        var range = Xlsx.utils.decode_range(this.worksheet["!ref"]);
        this._maxRow = range.e.r;
        this._maxColumn = range.e.c;
    }

    get maxRow() {
        return this._maxRow;
    }

    get maxColumn() {
        return this._maxColumn;
    }

    value(pos) {
        if (!this.worksheet[pos])
            return "";

        return this.worksheet[pos].v;
    }
};

module.exports = {
    Webo : Webo,
    WeboBook : WeboBook,
    Key : Key
};
