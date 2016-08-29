#include "halley/tools/tasks/editor_task.h"
#include "halley/text/halleystring.h"
#include <mutex>
#include <gsl/gsl_assert>
#include "halley/concurrency/concurrent.h"

using namespace Halley;

EditorTask::EditorTask(String name, bool isCancellable, bool isVisible) 
	: progress(0)
	, name(name)
	, cancelled(false)
	, pendingTaskCount(0)
	, isCancellable(isCancellable)
	, isVisible(isVisible)
{}

void EditorTask::addContinuation(EditorTaskAnchor&& task)
{
	continuations.emplace_back(std::move(task));
}

void EditorTask::setContinuations(Vector<EditorTaskAnchor>&& tasks)
{
	continuations = std::move(tasks);
}

void EditorTask::setProgress(float p, String label)
{
	std::lock_guard<std::mutex> lock(mutex);
	progress = std::max(0.0f, std::min(p, 1.0f));
	progressLabel = label;
}

bool EditorTask::isCancelled() const
{
	return cancelled;
}

bool EditorTask::hasPendingTasks() const
{
	return pendingTaskCount != 0;
}

void EditorTask::addPendingTask(EditorTaskAnchor&& task)
{
	std::lock_guard<std::mutex> lock(mutex);
	++pendingTaskCount;
	task.setParent(*this);
	pendingTasks.emplace_back(std::move(task));
}

void EditorTask::onPendingTaskDone(const EditorTaskAnchor& editorTaskAnchor)
{
	std::lock_guard<std::mutex> lock(mutex);
	--pendingTaskCount;
}

EditorTaskAnchor::EditorTaskAnchor(std::unique_ptr<EditorTask> t, float delay)
	: task(std::move(t))
	, status(EditorTaskStatus::WaitingToStart)
	, timeToStart(delay)
{
	Expects(!!task);
}

EditorTaskAnchor::EditorTaskAnchor(EditorTaskAnchor&& other) = default;

EditorTaskAnchor::~EditorTaskAnchor()
{
	// If this has been moved, task will be null
	if (task) {
		// Wait for task to join
		if (status != EditorTaskStatus::Done) {
			cancel();
			while (status == EditorTaskStatus::Started && !taskFuture.hasValue()) {}
		}

		if (parent) {
			parent->onPendingTaskDone(*this);
		}
	}
}

EditorTaskAnchor& EditorTaskAnchor::operator=(EditorTaskAnchor&& other) = default;

void EditorTaskAnchor::update(float time)
{
	if (status == EditorTaskStatus::WaitingToStart) {
		timeToStart -= time;
		if (timeToStart <= 0) {
			taskFuture = Concurrent::execute(Task<void>([this]() { task->run(); }));
			status = EditorTaskStatus::Started;
		}
	} else if (status == EditorTaskStatus::Started) {
		bool done = taskFuture.hasValue();
		if (done) {
			status = EditorTaskStatus::Done;
			progress = 1;
			progressLabel = "";
		} else {
			std::lock_guard<std::mutex> lock(task->mutex);
			progress = task->progress;
			progressLabel = task->progressLabel;
		}
	}
}

String EditorTaskAnchor::getName() const
{
	return task->name;
}

String EditorTaskAnchor::getProgressLabel() const
{
	return progressLabel;
}

bool EditorTaskAnchor::canCancel() const
{
	return task->isCancellable;
}

bool EditorTaskAnchor::isVisible() const
{
	return task->isVisible;
}

void EditorTaskAnchor::cancel()
{
	if (status == EditorTaskStatus::WaitingToStart) {
		status = EditorTaskStatus::Done;
	}
	if (task->isCancellable) {
		task->cancelled = true;
	}
}

Vector<EditorTaskAnchor> EditorTaskAnchor::getContinuations()
{
	return std::move(task->continuations);
}

Vector<EditorTaskAnchor> EditorTaskAnchor::getPendingTasks()
{
	if (task->pendingTaskCount > 0) {
		std::lock_guard<std::mutex> lock(task->mutex);
		return std::move(task->pendingTasks);
	} else {
		return Vector<EditorTaskAnchor>();
	}
}

void EditorTaskAnchor::setParent(EditorTask& editorTask)
{
	parent = &editorTask;
}
