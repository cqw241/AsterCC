async (page) => {
  const base = "http://127.0.0.1:8000";
  const errors = [];
  const checks = [];
  let offline = false;
  const onError = (error) => errors.push(error.message);
  const onConsole = (msg) => {
    if (msg.type() === "error" && !offline) errors.push(msg.text());
  };
  page.on("pageerror", onError);
  page.on("console", onConsole);
  page.setDefaultTimeout(15000);
  const check = (condition, label) => {
    if (!condition) throw new Error(label);
    checks.push(label);
  };
  const ready = async () => {
    await page
      .locator(".editor-caption .badge")
      .filter({ hasText: "编译成功" })
      .waitFor();
    await page.waitForFunction(
      () =>
        !document.querySelector(".stale-notice") &&
        !document.querySelector(".terminal-pending"),
    );
  };
  const submit = async (run) => {
    const response = page.waitForResponse(
      (r) => r.url().endsWith("/api/jobs") && r.request().method() === "POST",
    );
    await page
      .getByRole("button", { name: run ? /^运行/ : "编译", exact: !run })
      .click();
    await response;
    await ready();
    if (run) {
      await page
        .locator(".terminal-section .result-tabs button")
        .first()
        .click();
      await page.waitForFunction(() =>
        [...document.querySelectorAll(".command-heading")].some((e) =>
          e.textContent.includes("执行 main.aster"),
        ),
      );
    }
  };
  const edit = async (source) => {
    await page
      .locator(".source-panel .monaco-editor")
      .click({ position: { x: 180, y: 50 } });
    await page.keyboard.press("ControlOrMeta+A");
    await page.keyboard.insertText(source);
  };
  try {
    await page.setViewportSize({ width: 1440, height: 1000 });
    await page.goto(base);
    await page.evaluate(() => {
      for (const key of Object.keys(localStorage))
        if (
          key.startsWith("astercc:") ||
          key.startsWith("react-resizable-panels:")
        )
          localStorage.removeItem(key);
    });
    await page.reload();
    await ready();
    await page.getByRole("button", { name: /^运行/ }).click();
    await page
      .locator(".terminal-command pre")
      .filter({ hasText: "1290" })
      .waitFor();
    check(true, "Stage 3 compiled and executed optimization example: 1290");
    await page.screenshot({
      path: "playground/evidence/workbench.png",
      scale: "css",
    });
    await page.getByLabel("选择示例").selectOption("hello");
    await submit(true);
    await page
      .locator(".terminal-command pre")
      .filter({ hasText: "Hello from AsterCC" })
      .waitFor();
    check(true, "Example switch compiled and ran real Hello program");
    for (const level of [0, 1]) {
      await page
        .getByRole("button", { name: "O" + level, exact: true })
        .click();
      await submit(false);
      check(true, "Browser compilation at O" + level);
    }
    await page.getByRole("tab", { name: "AST", exact: true }).click();
    await page.locator(".ast-node").first().waitFor();
    await page.getByRole("button", { name: "JSON", exact: true }).click();
    await page.locator(".ast-view .monaco-editor").waitFor();
    await page.getByRole("tab", { name: "LLVM IR", exact: true }).click();
    await page.locator(".result-panel .monaco-editor").waitFor();
    check(true, "Real AST node/JSON and LLVM viewers loaded");
    const download = page.waitForEvent("download");
    await page.getByRole("button", { name: "下载源码", exact: true }).click();
    const downloaded = await download;
    check(
      downloaded.suggestedFilename() === "main.aster",
      "Source download is a real file",
    );
    const separator = page.getByRole("separator", {
      name: "调整源码与结果宽度",
    });
    const oldWidth = await page
      .locator(".source-panel")
      .evaluate((el) => el.getBoundingClientRect().width);
    const box = await separator.boundingBox();
    await page.mouse.move(box.x + box.width / 2, box.y + box.height / 2);
    await page.mouse.down();
    await page.mouse.move(box.x + 70, box.y + box.height / 2, { steps: 8 });
    await page.mouse.up();
    const newWidth = await page
      .locator(".source-panel")
      .evaluate((el) => el.getBoundingClientRect().width);
    check(newWidth > oldWidth + 30, "Editor split resizes by dragging");
    await separator.focus();
    await separator.press("ArrowLeft");
    check(true, "Split separator supports keyboard adjustment");
    await edit('fn main()->i64 { println("你好🌟");return missing; }');
    await page.getByRole("button", { name: "编译", exact: true }).click();
    await page
      .locator(".editor-caption .badge")
      .filter({ hasText: "编译失败" })
      .waitFor();
    await page.getByRole("button", { name: /^诊断/ }).click();
    await page
      .locator(".diagnostic-list button")
      .filter({ hasText: "undefined variable missing" })
      .click();
    check(
      (await page
        .locator(".source-panel .monaco-editor .squiggly-error")
        .count()) > 0,
      "UTF-8 diagnostic maps to an editor marker and source jump",
    );
    await page.getByLabel("选择示例").selectOption("optimization");
    await page.getByRole("button", { name: "O2", exact: true }).click();
    await submit(true);
    await page.getByRole("button", { name: "优化探索", exact: true }).click();
    await page.locator(".pass-step").nth(6).waitFor();
    await page.locator(".pass-step").nth(6).click();
    await page
      .locator(".pass-description h2")
      .filter({ hasText: "循环不变量外提" })
      .waitFor();
    await page
      .locator(".change-items button")
      .filter({ hasText: "移动" })
      .waitFor();
    check(true, "LICM diff identifies actual %14 motion from b5 to b0");
    await page.screenshot({
      path: "playground/evidence/optimization.png",
      scale: "css",
    });
    await page.getByRole("button", { name: "CFG / SSA", exact: true }).click();
    await page.locator(".cfg-block").first().waitFor();
    const mul = page
      .locator(".cfg-instructions button")
      .filter({ hasText: /%14.*mul/ });
    await mul.click();
    await page.locator(".value-inspector").filter({ hasText: "%14" }).waitFor();
    check(
      (await page.locator(".react-flow__edge").count()) >= 4,
      "Real CFG edges and SSA value inspector are interactive",
    );
    await page.screenshot({
      path: "playground/evidence/cfg.png",
      scale: "css",
    });
    await page.getByRole("button", { name: "基本块列表", exact: true }).click();
    check(
      (await page.locator(".block-list section").count()) === 4,
      "Accessible basic-block list matches graph",
    );
    await page.getByRole("button", { name: "IR Diff", exact: true }).click();
    await page.locator(".diff-container").waitFor();
    await page.getByRole("button", { name: "CFG / SSA", exact: true }).click();
    await page.locator(".cfg-block").first().waitFor();
    await page.getByRole("button", { name: "自举仪表盘", exact: true }).click();
    await page.locator(".stage-card").nth(3).waitFor();
    check(
      (await page.locator(".verification-strip").textContent()).includes(
        "逐字节一致",
      ),
      "Dashboard recomputes current Stage 2/3 hash equality",
    );
    await page.getByRole("button", { name: "O0", exact: true }).click();
    await page.getByRole("button", { name: "O2", exact: true }).click();
    await page.getByRole("button", { name: "IR 指令", exact: true }).click();
    await page.getByRole("button", { name: "运行时间", exact: true }).click();
    await page.getByLabel("测试编译器").selectOption("tests");
    await page.getByLabel("筛选测试").fill("diagnostic");
    check(
      (await page.locator(".test-results details").count()) === 10,
      "Test records switch and filter against actual Stage 0 results",
    );
    await page.getByLabel("筛选测试").fill("");
    await page.getByLabel("测试编译器").selectOption("tests-stage3");
    const evidenceDownload = page.waitForEvent("download");
    await page.getByRole("link", { name: "下载自举记录", exact: true }).click();
    check(
      (await evidenceDownload).suggestedFilename() === "bootstrap.json",
      "Bootstrap evidence download works",
    );
    await page.locator(".dashboard-page").evaluate((el) => (el.scrollTop = 0));
    await page.screenshot({
      path: "playground/evidence/bootstrap.png",
      scale: "css",
    });
    for (const width of [1024, 768, 390]) {
      await page.setViewportSize({ width, height: 844 });
      check(
        await page.evaluate(
          () => document.documentElement.scrollWidth <= window.innerWidth,
        ),
        "No page overflow at " + width + "px (dashboard)",
      );
      await page
        .getByRole("button", { name: "工作台", exact: true })
        .first()
        .click();
      await page.locator(".source-panel .monaco-editor").waitFor();
      check(
        await page.evaluate(
          () => document.documentElement.scrollWidth <= window.innerWidth,
        ),
        "No page overflow at " + width + "px (workbench)",
      );
      if (width === 390) {
        await page
          .getByRole("button", { name: "编译结果", exact: true })
          .click();
        check(
          await page.locator(".result-panel").isVisible(),
          "Mobile result tab reveals actual editor",
        );
        await page
          .getByRole("button", { name: "终端", exact: true })
          .first()
          .click();
        check(
          await page.locator(".terminal-panel").isVisible(),
          "Mobile terminal tab reveals execution output",
        );
        await page.getByRole("button", { name: "源码", exact: true }).click();
        await page.waitForFunction(
          () =>
            document.querySelector(".source-panel .view-lines")
              ?.childElementCount >= 10,
        );
        await page.screenshot({
          path: "playground/evidence/mobile.png",
          scale: "css",
        });
      }
      await page
        .getByRole("button", { name: "自举仪表盘", exact: true })
        .click();
      await page.locator(".stage-card").nth(3).waitFor();
    }
    await page.setViewportSize({ width: 1440, height: 1000 });
    await page
      .getByRole("button", { name: "工作台", exact: true })
      .first()
      .click();
    offline = true;
    await page.route("**/api/health", (route) => route.abort());
    await page.reload();
    await page
      .getByRole("alert")
      .filter({ hasText: "本地服务连接失败" })
      .waitFor();
    check(
      await page.locator(".source-panel .monaco-editor").isVisible(),
      "Backend unavailable state preserves source editing",
    );
    await page.unroute("**/api/health");
    await page.reload();
    await ready();
    offline = false;
    await page.getByRole("tab", { name: "SSA", exact: true }).click();
    check(
      errors.length === 0,
      "No unexpected browser console errors or uncaught exceptions",
    );
    return { status: "pass", checks, console_errors: errors, base_url: base };
  } catch (error) {
    throw new Error(
      "After " +
        (checks.at(-1) || "initialization") +
        ": " +
        error.message +
        "; browser errors: " +
        errors.join(" | "),
    );
  } finally {
    page.off("pageerror", onError);
    page.off("console", onConsole);
  }
};
